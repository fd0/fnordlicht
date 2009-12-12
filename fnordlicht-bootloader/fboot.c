/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlichtmini serial bootloader
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr/io.h>
#include <avr/boot.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/crc16.h>
#include <string.h>
#include <stdbool.h>
#include "../common/io.h"
#include "../common/remote-proto.h"
#include "../common/common.h"
#include "uart.h"
#include "global.h"

struct global_t
{
    uint8_t synced;
    uint8_t sync_len;

    union {
        uint8_t buf[REMOTE_MSG_LEN];
        struct remote_msg_t msg;
    };
    uint8_t len;

    uint8_t address;

    union {
        uint8_t data_buf[CONFIG_BOOTLOADER_BUFSIZE];
        uint16_t data_buf16[CONFIG_BOOTLOADER_BUFSIZE/2];
    };
    uint16_t data_len;
    uint8_t *data_address;

    bool crc_match;
    uint8_t delay;

    bool request_exit;
    uint8_t exit_delay;
};

struct global_t global;


/* disable watchgo - NEVER CALL DIRECTLY! */
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void disable_watchdog(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void disable_watchdog(void)
{
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

/* enter application */
void jump_to_application(void) __attribute__((naked, noinline));
void jump_to_application(void)
{
    /* jump to application reset vector */
    asm("ldi r30, 0");
    asm("ldi r31, 0");
    asm("icall");
}


static void start_application(void)
{
    /* pwm pins */
    P_PORT = 0;
    P_DDR = 0;

    /* in pin */
    R_PORT = 0;
    R_DDR = 0;

    /* uart */
    _UBRRH_UART0 = 0;
    _UBRRL_UART0 = 0;
    _UCSRA_UART0 = _BV(_TXC_UART0);
    _UCSRB_UART0 = 0;

    /* timer1 */
    TCCR1B = 0;
    OCR1A = 0;
    TCNT1 = 0;
    _TIFR_TIMER1 = 0xff;

    /* move interrupt vectors and start real application */
    jump_to_application();
}

static void parse_boot_config(struct remote_msg_boot_config_t *msg)
{
    /* remember flash address */
    global.data_address = (uint8_t *)msg->start_address;
}

static void parse_data(struct remote_msg_boot_data_t *msg)
{
    uint8_t len = sizeof(msg->data);
    if (global.data_len + len > CONFIG_BOOTLOADER_BUFSIZE)
        len = CONFIG_BOOTLOADER_BUFSIZE - global.data_len;

    memcpy(&global.data_buf[global.data_len], msg->data, len);
    global.data_len += len;
}

static void parse_crc(struct remote_msg_boot_crc_check_t *msg)
{
    /* compute crc16 over buffer */
    uint16_t checksum = 0xffff;

    uint8_t *ptr = &global.data_buf[0];
    uint16_t i = msg->len+1;
    while (--i)
        checksum = _crc16_update(checksum, *ptr++);

    if (checksum != msg->checksum) {
        global.delay = msg->delay;
        global.crc_match = false;
    } else
        global.crc_match = true;
}

static void parse_crc_flash(struct remote_msg_boot_crc_flash_t *msg)
{
    /* compute crc16 over flash */
    uint16_t checksum = 0xffff;

    uint8_t *address = (uint8_t *)msg->start;
    uint16_t i = msg->len+1;
    while (--i) {
        uint8_t data = pgm_read_byte(address++);
        checksum = _crc16_update(checksum, data);
    }

    if (checksum != msg->checksum) {
        global.delay = msg->delay;
        global.crc_match = false;
    } else
        global.crc_match = true;
}

static void flash(void)
{
    /* pull int */
    R_DDR |= _BV(INTPIN);

    uint8_t *addr = global.data_address;
    uint16_t *data = &global.data_buf16[0];

    uint8_t last_page = global.data_len/SPM_PAGESIZE;
    if (global.data_len % SPM_PAGESIZE)
        last_page++;

    for (uint8_t page = 0; page < last_page; page++)
    {
        PWM_PIN_ON(PWM_GREEN);

        /* erase page */
        boot_page_erase(addr);
        boot_spm_busy_wait();

        for (uint16_t i = 0; i < SPM_PAGESIZE; i += 2) {
            /* fill internal buffer */
            boot_page_fill(addr+i, *data++);
            boot_spm_busy_wait();
        }

        /* after filling the temp buffer, write the page and wait till we're done */
        boot_page_write(addr);
        boot_spm_busy_wait();

        /* re-enable application flash section, so we can read it again */
        boot_rww_enable();

        PWM_PIN_OFF(PWM_GREEN);
        addr += SPM_PAGESIZE;
    }

    global.data_address = addr;

    /* release int */
    R_DDR &= ~_BV(INTPIN);
}

static void remote_parse_msg(struct remote_msg_t *msg)
{
    /* verify address */
    if (msg->address != global.address && msg->address != REMOTE_ADDR_BROADCAST)
        return;

    /* parse command */
    switch (msg->cmd) {
        case REMOTE_CMD_BOOT_CONFIG: parse_boot_config((struct remote_msg_boot_config_t *)msg);
                                     break;
        case REMOTE_CMD_BOOT_INIT:   global.data_len = 0;
                                     break;
        case REMOTE_CMD_BOOT_DATA:   parse_data((struct remote_msg_boot_data_t *)msg);
                                     break;
        case REMOTE_CMD_CRC_CHECK:   parse_crc((struct remote_msg_boot_crc_check_t *)msg);
                                     break;
        case REMOTE_CMD_CRC_FLASH:   parse_crc_flash((struct remote_msg_boot_crc_flash_t *)msg);
                                     break;
        case REMOTE_CMD_FLASH:       flash();
                                     break;
        case REMOTE_CMD_ENTER_APP:   global.request_exit = true;
                                     global.exit_delay = CONFIG_EXIT_DELAY;
                                     break;
    }

    if (global.delay && global.crc_match == false) {
        /* pull int to gnd */
        R_DDR |= _BV(INTPIN);

        PWM_PIN_ON(PWM_RED);
    }
}

static void remote_poll(void)
{
    /* wait for a byte */
    if (uart_receive_complete())
    {
        uint8_t data = uart_getc();

        /* check if sync sequence has been received before */
        if (global.sync_len == REMOTE_SYNC_LEN) {
            /* synced, safe address and send next address to following device */
            global.address = data;
            uart_putc(data+1);

            /* reset buffer */
            global.len = 0;

            /* enable remote command thread */
            global.synced = 1;
        } else {
            /* just pass through data */
            uart_putc(data);

            /* put data into remote buffer
             * (just processed if remote.synced == 1) */
            if (global.len < sizeof(global.buf))
                global.buf[global.len++] = data;
        }

        /* remember the number of sync bytes received so far */
        if (data == REMOTE_CMD_RESYNC)
            global.sync_len++;
        else
            global.sync_len = 0;
    }

    if (global.synced && global.len == REMOTE_MSG_LEN) {
        remote_parse_msg(&global.msg);
        global.len = 0;
    }
}

static void check_startup(void)
{
    /* check for valid reset vector */
    if (pgm_read_word(NULL) == 0xffff)
        return;

    /* configure pullup resistor at int pin */
    R_PORT |= _BV(INTPIN);

    /* sleep 100ms */
    for (uint8_t i = 0; i < CONFIG_DELAY; i++)
        _delay_loop_2(CONFIG_DELAY_LOOP2);

    /* if int pin is pulled down, remain in bootloader */
    if (!(R_PIN & _BV(INTPIN)))
        return;

    /* els cleanup and start application */
    start_application();
}

int __attribute__ ((noreturn,OS_main)) main(void)
{
    /* immediately start application, if the reset has been caused
     * by a watchdog reset */
    if (mcusr_mirror & _BV(WDRF))
        jump_to_application();

    /* start application if necessary */
    check_startup();

    /* configure and enable uart */
    uart_init();

    /* configure outputs */
    P_DDR |= PWM_CHANNEL_MASK;

#ifdef PWM_INVERTED
    P_PORT |= PWM_CHANNEL_MASK;
#else
    P_PORT &= ~PWM_CHANNEL_MASK;
#endif

    /* initialize timer1, CTC at 50ms, prescaler 1024 */
    OCR1A = F_CPU/1024/20;
    TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10);

    while(1) {
        remote_poll();

        if (_TIFR_TIMER1 & _BV(OCF1A)) {
            _TIFR_TIMER1 = _BV(OCF1A);

            static uint8_t c;
            if (c == 20) {
                /* blink */
                PWM_PIN_TOGGLE(PWM_BLUE);
                c = 0;
            } else
                c++;

            /* if int is pulled, decrement delay */
            if (R_DDR & _BV(INTPIN)) {
                if (--global.delay == 0) {
                    /* release int */
                    R_DDR &= ~_BV(INTPIN);
                    P_PORT &= ~1;
                }
            }

            /* exit (after exit_delay) if requested */
            if (global.request_exit && global.exit_delay-- == 0)
                start_application();
        }
    }
}
