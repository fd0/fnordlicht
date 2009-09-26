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
#include "pwm.h"
#include "pt/pt.h"
#include "script.h"

#define REMOTE_MSGLEN 10

#define REMOTE_CMD_SET 0x01
#define REMOTE_CMD_FADE 0x02
#define REMOTE_CMD_CONFIG 0x03
#define REMOTE_CMD_RESYNC 0x1b

#define REMOTE_ADDR_BROADCAST 0xff

struct remote_msg_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t data[REMOTE_MSGLEN-2];
};

struct remote_msg_fade
{
    uint8_t address;
    uint8_t cmd;
    uint16_t speed;
    uint8_t target[PWM_CHANNELS];
};

struct remote_msg_config
{
    uint8_t address;
    uint8_t cmd;
    uint8_t scripting;
};

struct remote_state_t
{
    union {
        struct remote_msg_t msg;
        uint8_t buf[REMOTE_MSGLEN];
    };
    uint8_t buflen;

    uint8_t sync;
    uint8_t synced;
    uint8_t address;
    struct pt thread;
};

struct remote_state_t remote;

#if CONFIG_SERIAL && CONFIG_REMOTE

void remote_init(void)
{
    /* initialize struct */
    remote.buflen = 0;
    remote.sync = 0;
    remote.synced = 0;
    remote.address = 0;
}

static void remote_parse_msg(struct remote_msg_t *msg)
{
    /* verify address */
    if (msg->address != remote.address && msg->address != REMOTE_ADDR_BROADCAST)
        return;

    if (msg->cmd == REMOTE_CMD_SET) {
        for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
            global_pwm.channels[i].brightness = msg->data[i];
            global_pwm.channels[i].target_brightness = msg->data[i];
        }

        return;
    }

    if (msg->cmd == REMOTE_CMD_FADE) {
        struct remote_msg_fade *m = (struct remote_msg_fade *)msg;

        /* set new color, without fading */
        for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
            global_pwm.channels[i].speed = m->speed;
            global_pwm.channels[i].target_brightness = m->target[i];
        }

        return;
    }

    if (msg->cmd == REMOTE_CMD_CONFIG) {
        struct remote_msg_config *m = (struct remote_msg_config *)msg;

        /* enable or disable scripting */
        if (m->scripting)
            script_global.enable = 1;
        else
            script_global.enable = 0;
        return;
    }
}

static PT_THREAD(remote_thread(struct pt *thread))
{
    PT_BEGIN(thread);

    while (1) {
        PT_WAIT_UNTIL(thread, remote.buflen == REMOTE_MSGLEN);

        remote_parse_msg(&remote.msg);
        remote.buflen = 0;
    }

    PT_END(thread);
}

void remote_poll(void)
{
    if (fifo_fill(&global_uart.rx) > 0) {
        uint8_t data = fifo_dequeue(&global_uart.rx);

        /* check if sync sequence has been received before */
        if (remote.sync == REMOTE_MSGLEN-1) {
            /* synced, safe address and send next address to following device */
            remote.address = data;
            uart_putc(data+1);

            /* reset remote buffer */
            remote.buflen = 0;

            /* enable remote command thread */
            remote.synced = 1;
            PT_INIT(&remote.thread);
        } else {
            /* just pass through data */
            uart_putc(data);

            /* put data into remote buffer
             * (just processed if remote.synced == 1) */
            if (remote.buflen < sizeof(remote.buf))
                remote.buf[remote.buflen++] = data;
        }

        /* remember the number of sync bytes received so far */
        if (data == REMOTE_CMD_RESYNC)
            remote.sync++;
        else
            remote.sync = 0;
    }

    if (remote.synced)
        PT_SCHEDULE(remote_thread(&remote.thread));
}

#endif
