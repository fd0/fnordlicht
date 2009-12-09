#include <avr/io.h>
#include <avr/boot.h>
#include <util/delay.h>
#include <string.h>
#include "../config.h"
#include "../pt/pt.h"
#include "../remote-proto.h"

#ifndef CONFIG_BOOTLOADER_BUFSIZE
#define CONFIG_BOOTLOADER_BUFSIZE 512
#endif

struct global_t
{
    union {
        uint8_t buf[REMOTE_MSGLEN];
        struct remote_msg_t msg;
    };
    uint8_t len;

    enum mem_type_t mem;
    uint16_t address;
    uint16_t bytes_remaining;

    uint8_t data_buf[CONFIG_BOOTLOADER_BUFSIZE];
    uint16_t data_len;
};

struct global_t global;

/* define uart baud rate (19200) and mode (8N1) */
#if defined(__AVR_ATmega8__)
/* in atmega8, we need a special switching bit
 * for addressing UCSRC */
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)

#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__)
/* in atmega88, this isn't needed any more */
#define UART_UCSRC _BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0)
#endif

#if CONFIG_SERIAL_BAUDRATE == 115200 && F_CPU == 16000000UL
    #define UART_UBRR 8
#elif CONFIG_SERIAL_BAUDRATE == 115200 && F_CPU == 20000000UL
    #define UART_UBRR 10
#else
    #if CONFIG_SERIAL_BAUDRATE > 57600
    #warn "high uart baudrate, UART_UBRR might not be correct!"
    #endif
    #define UART_UBRR (F_CPU/(CONFIG_SERIAL_BAUDRATE * 16L)-1)
#endif

/* parse message */
static void parse_message(struct remote_msg_t *msg)
{
    if (msg->cmd == REMOTE_CMD_BOOT_CONFIG) {
        struct remote_msg_boot_config *m = (struct remote_msg_boot_config *)msg;

        global.mem = m->mem;
        global.address = m->start_address;
        global.bytes_remaining = m->length;

        /* do a chip erase if flash should be written */
        if (global.mem == MEM_FLASH) {
            /* iterate over all pages in flash, and try to erase every single
             * one of them (the bootloader section should be protected by lock-bits (!) */
            for (uint16_t addr = 0; addr < BOOT_SECTION_START; addr += SPM_PAGESIZE)
                boot_page_erase(addr);
        }

        return;
    }

    if (msg->cmd == REMOTE_CMD_BOOT_FLASH) {
        struct remote_msg_boot_flash *m = (struct remote_msg_boot_flash *)msg;

        for (uint8_t i = 0;
             i < REMOTE_MSGLEN-2 && global.data_len < CONFIG_BOOTLOADER_BUFSIZE;
             i++) {
            global.data_buf[global.data_len++] = m->data[i];
            global.bytes_remaining--;
        }

        return;
    }
}


/* initialize and cleanup uart (taken from ../uart.c) */
static inline void init_uart(void)
{
    /* set baud rate */
    _UBRRH_UART0 = (uint8_t)(UART_UBRR >> 8);  /* high byte */
    _UBRRL_UART0 = (uint8_t)UART_UBRR;         /* low byte */

    /* set mode */
    _UCSRC_UART0 = UART_UCSRC;

    /* enable transmitter and receiver */
    _UCSRB_UART0 = _BV(_TXEN_UART0) | _BV(_RXEN_UART0);
}

static inline void cleanup_uart(void)
{
    _UBRRH_UART0 = 0;
    _UBRRL_UART0 = 0;
    _UCSRC_UART0 = 0;
    _UCSRB_UART0 = 0;
}

static PT_THREAD(remote_thread(struct pt *thread))
{
    static uint16_t temp_addr;
    PT_BEGIN(thread);

    while (1) {
        /* wait for enough chars for a message */
        PT_WAIT_UNTIL(thread, _UCSRA_UART0 & _BV(_RXC_UART0));

        /* read data */
        global.buf[global.len++] = UDR;

        /* parse message */
        if (global.len == REMOTE_MSGLEN) {
            parse_message(&global.msg);
            global.len = 0;
        }

        /* start flashing, if we have enough bytes */
        if (global.data_len >= SPM_PAGESIZE && !boot_spm_busy()) {
            PWM_PORT ^= _BV(PB2);

            /* fill bytes */
            temp_addr = global.address;
            for (uint8_t i = 0; i < SPM_PAGESIZE; i+=2) {
                boot_page_fill(temp_addr, global.data_buf[i] | global.data_buf[i+1] << 8);
                temp_addr += 2;
            }

            /* after filling the temp buffer, write the page and wait till we're done */
            boot_page_write_safe(global.address);

            /* re-enable application flash section, so we can read it again */
            // boot_rww_enable();

            /* store next page's address, since we do auto-address-incrementing */
            global.address = temp_addr;

            /* move data in buffer around */
            global.data_len -= SPM_PAGESIZE;
            memmove(&global.data_buf[0], &global.data_buf[SPM_PAGESIZE], global.data_len);
        }
    }

    PT_END(thread);
}

int main(void)
{
    /* init structs */
    struct pt pt;
    PT_INIT(&pt);
    global.len = 0;
    global.bytes_remaining = 0;

    /* configure and enable uart */
    init_uart();

    /* configure output pins */
    PWM_DDR = PWM_CHANNEL_MASK;

    /* initialize timer1, CTC at 500ms, prescaler 1024 */
    OCR1A = F_CPU/1024/4;
    TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10);

    while(1) {
        /* switch output every timer interrupt */
        if (TIFR & _BV(OCF1A)) {
            PWM_PORT ^= _BV(PB0);
            TIFR = _BV(OCF1A);
        }

        /* call remote command parser thread until thread exited */
        if (PT_SCHEDULE(remote_thread(&pt)) == 0)
            break;
    }

    /* cleanup */
    cleanup_uart();
    PWM_DDR = 0;
    PWM_PORT = 0;
    TCCR1B = 0;
    OCR1A = 0;
    TIFR = TIFR;
}
