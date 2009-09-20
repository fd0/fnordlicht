/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include "config.h"
#include "remote.h"
#include "fifo.h"
#include "uart.h"
#include "static_scripts.h"
#include "pwm.h"
#include "pt/pt.h"

#define STX 0x02
#define ETX 0x03

#define REMOTE_CMD_SET 0x01
#define REMOTE_CMD_RESYNC 0x1b

#define REMOTE_MSGLEN 10

#define REMOTE_ADDR_BROADCAST 0xff

struct remote_msg_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t data[REMOTE_MSGLEN-2];
};

struct remote_state_t
{
    union {
        struct remote_msg_t msg;
        uint8_t buf[REMOTE_MSGLEN];
    };
    uint8_t buflen;
    uint8_t sync_counter;
    uint8_t initialized;
    uint8_t address;
    struct pt thread;
};

struct remote_state_t remote;

#if SERIAL_UART

void remote_init(void)
{
    /* initialize struct */
    remote.sync_counter = 0;
    remote.initialized = 0;
    remote.address = 0;
    remote.buflen = 0;

    PT_INIT(&remote.thread);
}

static void remote_parse_msg(struct remote_msg_t *msg)
{
    /* ignore sync attempts */
    if (msg->cmd == REMOTE_CMD_RESYNC)
        return;

    /* first, verify address */
    if (msg->address != remote.address && msg->address != REMOTE_ADDR_BROADCAST)
        return;

    /* check opcode */
    if (msg->cmd == REMOTE_CMD_SET) {
        for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
            global_pwm.channels[i].brightness = msg->data[i];
            global_pwm.channels[i].target_brightness = msg->data[i];
        }
    }
}

static PT_THREAD(remote_thread(struct pt *thread))
{
    PT_BEGIN(thread);

    /* wait for REMOTE_MSGLEN-1 many esc characters */
    while (remote.sync_counter < REMOTE_MSGLEN-1) {
        /* wait for a byte and pass through */
        PT_WAIT_UNTIL(&remote.thread, fifo_fill(&global_uart.rx_fifo) > 0);
        uint8_t data = fifo_dequeue(&global_uart.rx_fifo);
        uart_putc(data);

        /* check for esc */
        if (data == 0x1b)
            remote.sync_counter++;
        else
            remote.sync_counter = 0;
    }

    /* next character is address */
    PT_WAIT_UNTIL(&remote.thread, fifo_fill(&global_uart.rx_fifo) > 0);
    uint8_t address = fifo_dequeue(&global_uart.rx_fifo);

    /* this is our address */
    remote.address = address;
    remote.initialized = 1;

    /* increment address and send to next device */
    uart_putc(++address);

    /* disable scripting */
    disable_script_threads();

#if DEBUG
    /* reset color */
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        global_pwm.channels[i].target_brightness = 0;
        global_pwm.channels[i].brightness = 0;
    }
#endif

    while (1) {
        while (remote.buflen < REMOTE_MSGLEN) {
            PT_WAIT_UNTIL(&remote.thread, fifo_fill(&global_uart.rx_fifo) > 0);
            uint8_t data = fifo_dequeue(&global_uart.rx_fifo);
            uart_putc(data);
            remote.buf[remote.buflen++] = data;
        }

        /* parse message and reset buffer */
        remote_parse_msg(&remote.msg);
        remote.buflen = 0;
    }

    PT_END(thread);
}

void remote_poll(void)
{
    PT_SCHEDULE(remote_thread(&remote.thread));
}

#endif
