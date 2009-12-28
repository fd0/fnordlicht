/*
 * unzap firmware
 *
 * (c) by Alexander Neumann <alexander@lochraster.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "globals.h"
#include "ui.h"
#include "timer.h"
#include "uart.h"
#include "../common/pt/pt.h"
#include "remote-proto.h"

/* module local variables */

static struct pt btn_thread;
/* buttons are idle at start (internal pullups active) */
static uint8_t btn_state = _BV(BTN1_PIN) | _BV(BTN2_PIN);
static uint8_t btn_last_sample = _BV(BTN1_PIN) | _BV(BTN2_PIN);
static uint8_t btn_press = 0;

static struct pt blink_thread;
static uint8_t blink_seq_led1 = 0;
static uint8_t blink_seq_led2 = 0;

static struct pt input_thread;

/* initialize the button and led pins */
void ui_init(void)
{
    /* initialize buttons */
    BTN_DDR &= ~(_BV(BTN1_PIN) | _BV(BTN2_PIN));
    BTN_PORT |= _BV(BTN1_PIN) | _BV(BTN2_PIN);

    /* initialize leds */
    LED_DDR |= _BV(LED1_PIN) | _BV(LED2_PIN);
    LED_PORT &= ~(_BV(LED1_PIN) | _BV(LED2_PIN));

    /* initialize button sample thread and variables */
    PT_INIT(&btn_thread);

    /* initialize blink thread and variables */
    PT_INIT(&blink_thread);

    /* initialize input thread */
    PT_INIT(&input_thread);
}

/* blink out a sequence (LSB first), every bit is 150ms long */
void ui_blink(uint8_t sequence1, uint8_t sequence2)
{
    blink_seq_led1 = sequence1;
    blink_seq_led2 = sequence2;
}

/* check if the current blink sequency is done */
bool ui_blinking(void)
{
    return !(blink_seq_led1 || blink_seq_led2);
}

/* sample buttons, set bit in btn_press if a button has been pressed */
static PT_THREAD(ui_sample_buttons(struct pt *thread))
{
    /* make sure no variable is created on the stack */
    static timer_t btn_timer;

    PT_BEGIN(thread);

    while(1) {
        /* only execute this thread every 10ms */
        timer_set(&btn_timer, 1);
        PT_WAIT_UNTIL(thread, timer_expired(&btn_timer));

        /* sample buttons */
        /* ATTENTION: make SURE, btn_sample is NOT created on the stack! */
        register uint8_t btn_sample = BTN_PIN & (_BV(BTN1_PIN) | _BV(BTN2_PIN));

        /* mark bits which stayed the same since last sample */
        btn_last_sample ^= ~btn_sample;

        /* mark bits which have not changed, but whose state is different */
        btn_last_sample = btn_state ^ (btn_sample & btn_last_sample);

        /* if old state is one (button not pressed), new state is zero (button pressed),
         * so set these bits in btn_press */
        btn_press |= btn_last_sample & btn_state;

        /* remember new state and last sample */
        btn_state ^= btn_last_sample;
        btn_last_sample = btn_sample;
    }

    PT_END(thread);
}

/* execute led blinking */
static PT_THREAD(ui_do_blinking(struct pt *thread))
{
    static timer_t t;

    PT_BEGIN(thread);

    while (1) {

        /* wait until there is something to do */
        PT_WAIT_UNTIL(thread, blink_seq_led1 || blink_seq_led2);

        /* turn leds on/off, according to blink sequence */
        if (blink_seq_led1 & 1)
            LED1_ON();
        else
            LED1_OFF();

        if (blink_seq_led2 & 1)
            LED2_ON();
        else
            LED2_OFF();

        blink_seq_led1 >>= 1;
        blink_seq_led2 >>= 1;

        /* wait 150ms */
        timer_set(&t, 15);
        PT_WAIT_UNTIL(thread, timer_expired(&t));

        /* turn off leds */
        LED1_OFF();
        LED2_OFF();

    }

    PT_END(thread);
}

#define MASTER_PROGRAMS 4
static PROGMEM uint8_t master_parameters[] = {
    /* colorwheel forward rainbow */
    0,          /* program index */
    5,          /* fade step */
    1,          /* fade delay */
    0,          /* fade sleep */
    0, 0,       /* hue start (little endian) */
    20, 0,      /* hue step (little endian) */
    -1,         /* addr add */
    255,        /* saturation */
    255,        /* value */

    /* colorwheel all same color */
    0,          /* program index */
    5,          /* fade step */
    1,          /* fade delay */
    0,          /* fade sleep */
    0, 0,       /* hue start (little endian) */
    20, 0,      /* hue step (little endian) */
    0,          /* addr add */
    255,        /* saturation */
    255,        /* value */

    /* colorwheel all same color, low saturation */
    0,          /* program index */
    5,          /* fade step */
    1,          /* fade delay */
    0,          /* fade sleep */
    0, 0,       /* hue start (little endian) */
    20, 0,      /* hue step (little endian) */
    0,          /* addr add */
    150,        /* saturation */
    255,        /* value */

    /* colorwheel all same color, low saturation */
    0,          /* program index */
    5,          /* fade step */
    1,          /* fade delay */
    0,          /* fade sleep */
    0, 0,       /* hue start (little endian) */
    120, 0,      /* hue step (little endian) */
    0,          /* addr add */
    255,        /* saturation */
    120,        /* value */
};

void start_program(uint8_t index)
{
    /* load parameters */
    union {
        uint8_t raw[REMOTE_MSG_LEN];
        struct remote_msg_start_program_t msg;
    } data;

    data.msg.address = 255;
    data.msg.cmd = REMOTE_CMD_START_PROGRAM;

    uint8_t *ptr = &master_parameters[index*REMOTE_STARTUP_MAX_PARAMSIZE];
    data.msg.script = pgm_read_byte(ptr++);

    for (uint8_t i = 0; i < REMOTE_STARTUP_MAX_PARAMSIZE; i++) {
        uint8_t d = pgm_read_byte(ptr++);
        data.msg.params[i] = d;
    }
    for (uint8_t i = REMOTE_STARTUP_MAX_PARAMSIZE; i < REMOTE_MSG_LEN-2; i++)
        data.msg.params[i] = 0;

    for (uint8_t i = 0; i < REMOTE_MSG_LEN; i++)
        uart_putc(data.raw[i]);

    ui_blink(0x01, 0);
}

static void send_resync(uint8_t addr)
{
    for (uint8_t i = 0; i < REMOTE_SYNC_LEN; i++)
        uart_putc(REMOTE_CMD_RESYNC);
    uart_putc(addr);
}

static void send_stop(uint8_t addr)
{
    /* load parameters */
    union {
        uint8_t raw[REMOTE_MSG_LEN];
        struct remote_msg_t msg;
    } data;

    data.msg.cmd = REMOTE_CMD_STOP;
    data.msg.address = addr;
    data.msg.data[0] = 1;

    for (uint8_t i = 0; i < REMOTE_MSG_LEN; i++)
        uart_putc(data.raw[i]);
}

/* parse input */
static PT_THREAD(ui_input(struct pt*thread))
{
    static timer_t t;
    static uint8_t option_set;
    static uint8_t current_program;

    PT_BEGIN(thread);

    current_program = 0;

    while(1) {

        if (btn_press & _BV(BTN1_PIN)) {
            PT_WAIT_UNTIL(thread, fifo_empty(&global_uart.tx));
            send_resync(0);
            PT_WAIT_UNTIL(thread, fifo_empty(&global_uart.tx));
            send_stop(255);
            PT_WAIT_UNTIL(thread, fifo_empty(&global_uart.tx));
            start_program(current_program);
            current_program = (current_program+1) % MASTER_PROGRAMS;
        }

        if (btn_press & _BV(BTN2_PIN)) {
            //uart_putc('2');
        }

        btn_press = 0;

        PT_YIELD(thread);
    }

    PT_END(thread);
}

/* poll for user actions */
void ui_poll(void)
{
    /* sample buttons */
    PT_SCHEDULE(ui_sample_buttons(&btn_thread));

    /* do blinking */
    PT_SCHEDULE(ui_do_blinking(&blink_thread));

    /* process input */
    PT_SCHEDULE(ui_input(&input_thread));
}
