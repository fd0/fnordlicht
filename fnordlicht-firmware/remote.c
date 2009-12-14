/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
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

#include <stdlib.h>
#include <string.h>
#include <avr/wdt.h>
#include "globals.h"
#include "remote.h"
#include "fifo.h"
#include "uart.h"
#include "pwm.h"
#include "../common/pt/pt.h"
#include "script.h"
#include "remote-proto.h"
#include "../common/common.h"
#include "storage.h"
#include "timer.h"

/* abbreviations for port, ddr and pin */
#define R_PORT _OUTPORT(REMOTE_INT_PORT)
#define R_DDR _DDRPORT(REMOTE_INT_PORT)
#define R_PIN _INPORT(REMOTE_INT_PORT)
#define INTPIN REMOTE_INT_PIN

struct remote_state_t
{
    /* serial communication */
    union {
        struct remote_msg_t msg;
        uint8_t buf[REMOTE_MSG_LEN];
    };
    uint8_t buflen;

    uint8_t sync;
    uint8_t synced;
    struct pt thread;

    /* int line */
    timer_t int_timer;
    enum {
        INT_IDLE = 0,
        INT_PULLED = 1,
        INT_PULLED_TIMER = 2,
    } int_state;
};

struct remote_state_t remote;
struct global_remote_t global_remote;

#if CONFIG_SERIAL && CONFIG_REMOTE

/* static parsing routines */
static void parse_fade_rgb(struct remote_msg_fade_rgb_t *msg);
static void parse_fade_hsv(struct remote_msg_fade_hsv_t *msg);
static void parse_save_rgb(struct remote_msg_save_rgb_t *msg);
static void parse_save_hsv(struct remote_msg_save_hsv_t *msg);
static void parse_save_current(struct remote_msg_save_current_t *msg);
static void parse_config_offsets(struct remote_msg_config_offsets_t *msg);
static void parse_start_program(struct remote_msg_start_program_t *msg);
static void parse_stop(struct remote_msg_stop_t *msg);
static void parse_modify_current(struct remote_msg_modify_current_t *msg);
static void parse_pull_int(struct remote_msg_pull_int_t *msg);
static void parse_config_startup(struct remote_msg_config_startup_t *msg);
static void parse_bootloader(struct remote_msg_bootloader_t *msg);

void remote_init(void)
{
    /* initialize offsets */
    global_remote.offsets.saturation = 255;
    global_remote.offsets.value = 255;

    /* initialize pin, tri-state */
    R_DDR &= ~_BV(INTPIN);
    R_PORT &= ~_BV(INTPIN);
    remote.int_state = INT_IDLE;
}

static void remote_parse_msg(struct remote_msg_t *msg)
{
    /* verify address */
    if (msg->address != global_remote.address && msg->address != REMOTE_ADDR_BROADCAST)
        return;

    /* parse command */
    switch (msg->cmd) {
        case REMOTE_CMD_FADE_RGB:
            parse_fade_rgb((struct remote_msg_fade_rgb_t *)msg);
            break;
        case REMOTE_CMD_FADE_HSV:
            parse_fade_hsv((struct remote_msg_fade_hsv_t *)msg);
            break;
        case REMOTE_CMD_SAVE_RGB:
            parse_save_rgb((struct remote_msg_save_rgb_t *)msg);
            break;
        case REMOTE_CMD_SAVE_HSV:
            parse_save_hsv((struct remote_msg_save_hsv_t *)msg);
            break;
        case REMOTE_CMD_SAVE_CURRENT:
            parse_save_current((struct remote_msg_save_current_t *)msg);
            break;
        case REMOTE_CMD_CONFIG_OFFSETS:
            parse_config_offsets((struct remote_msg_config_offsets_t *)msg);
            break;
        case REMOTE_CMD_START_PROGRAM:
            parse_start_program((struct remote_msg_start_program_t *)msg);
            break;
        case REMOTE_CMD_STOP:
            parse_stop((struct remote_msg_stop_t *)msg);
            break;
        case REMOTE_CMD_MODIFY_CURRENT:
            parse_modify_current((struct remote_msg_modify_current_t *)msg);
            break;
        case REMOTE_CMD_PULL_INT:
            parse_pull_int((struct remote_msg_pull_int_t *)msg);
            break;
        case REMOTE_CMD_CONFIG_STARTUP:
            parse_config_startup((struct remote_msg_config_startup_t *)msg);
            break;
        case REMOTE_CMD_BOOTLOADER:
            parse_bootloader((struct remote_msg_bootloader_t *)msg);
            break;

    }
}

static PT_THREAD(remote_thread(struct pt *thread))
{
    PT_BEGIN(thread);

    while (1) {
        PT_WAIT_UNTIL(thread, remote.buflen == REMOTE_MSG_LEN);

        remote_parse_msg(&remote.msg);
        remote.buflen = 0;
    }

    PT_END(thread);
}

void remote_poll(void)
{
    if (fifo_fill((fifo_t *)&global_uart.rx) > 0) {
        uint8_t data = fifo_dequeue((fifo_t *)&global_uart.rx);

        /* check if sync sequence has been received before */
        if (remote.sync == REMOTE_SYNC_LEN) {
            /* synced, safe address and send next address to following device */
            global_remote.address = data;
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

    /* check int timer */
    if (remote.int_state == INT_PULLED_TIMER && timer_expired(&remote.int_timer)) {
        remote_release_int();
    }
}

void remote_pull_int(void)
{
    if (remote.int_state != INT_IDLE)
        return;

    /* pull int pin to ground */
    R_DDR |= _BV(INTPIN);
    R_PORT &= ~_BV(INTPIN);
}

void remote_release_int(void)
{
    if (remote.int_state == INT_IDLE)
        return;

    /* reset pin to tri-state */
    R_DDR &= ~_BV(INTPIN);
    R_PORT &= ~_BV(INTPIN);
    remote.int_state = INT_IDLE;
}

/* offset helper functions */
uint8_t apply_offset(uint8_t value, int8_t offset)
{
    if (offset < 0) {
        if (value > -offset)
            value += offset;
        else
            value = 1;
    } else if (offset > 0) {
        uint16_t temp = value;
        temp += offset;
        if (temp > 255)
            value = 255;
        else
            value = LO8(temp);
    }

    return value;
}

static uint8_t apply_scale(uint8_t value, uint8_t scale)
{
    uint16_t temp = value * scale;
    temp /= 255;
    return LO8(temp);
}

void apply_hsv_offset(struct hsv_color_t *color)
{
    color->hue += global_remote.offsets.hue;
    color->saturation = apply_scale(color->saturation, global_remote.offsets.saturation);
    color->value = apply_scale(color->value, global_remote.offsets.value);
}

/* parser functions */

void parse_fade_rgb(struct remote_msg_fade_rgb_t *msg)
{
    uint8_t step = apply_offset(msg->step, global_remote.offsets.step);
    uint8_t delay = apply_offset(msg->delay, global_remote.offsets.delay);
    pwm_fade_rgb(&msg->color, step, delay);
}

void parse_fade_hsv(struct remote_msg_fade_hsv_t *msg)
{
    uint8_t step = apply_offset(msg->step, global_remote.offsets.step);
    uint8_t delay = apply_offset(msg->delay, global_remote.offsets.delay);
    apply_hsv_offset(&msg->color);
    pwm_fade_hsv(&msg->color, step, delay);
}

void parse_save_rgb(struct remote_msg_save_rgb_t *msg)
{
    if (msg->slot >= CONFIG_EEPROM_COLORS)
        return;

    struct storage_color_t c;
    c.step = msg->step;
    c.delay = msg->delay;
    c.pause = msg->pause;

    /* mark color as rgb */
    c.color.rgb_marker = 255;

    c.color.red = msg->color.red;
    c.color.green = msg->color.green;
    c.color.blue = msg->color.blue;

    storage_save_color(msg->slot, &c);
}

void parse_save_hsv(struct remote_msg_save_hsv_t *msg)
{
    if (msg->slot >= CONFIG_EEPROM_COLORS)
        return;

    struct storage_color_t c;
    c.step = msg->step;
    c.delay = msg->delay;
    c.pause = msg->pause;

    c.color.hue = msg->color.hue;
    c.color.saturation = msg->color.saturation;
    c.color.value = msg->color.value;

    storage_save_color(msg->slot, &c);
}

void parse_save_current(struct remote_msg_save_current_t *msg)
{
    if (msg->slot >= CONFIG_EEPROM_COLORS)
        return;

    struct storage_color_t c;
    c.step = msg->step;
    c.delay = msg->delay;
    c.pause = msg->pause;

    /* mark color as rgb */
    c.color.rgb_marker = 255;

    c.color.red = global_pwm.current.red;
    c.color.green = global_pwm.current.green;
    c.color.blue = global_pwm.current.blue;

    storage_save_color(msg->slot, &c);
}

void parse_config_offsets(struct remote_msg_config_offsets_t *msg)
{
    global_remote.offsets.step = msg->step;
    global_remote.offsets.delay = msg->delay;
    global_remote.offsets.hue = msg->hue;
    global_remote.offsets.saturation = msg->saturation;
    global_remote.offsets.value = msg->value;
}

void parse_start_program(struct remote_msg_start_program_t *msg)
{
    parse_stop(NULL);
    script_start(0, msg->script, (union program_params_t *)&msg->parameters);
}

void parse_stop(struct remote_msg_stop_t *msg)
{
    script_stop();
    if (msg->fade)
        pwm_stop_fading();
}

void parse_modify_current(struct remote_msg_modify_current_t *msg)
{
    /* return if a color change is in progress */
#if CONFIG_SCRIPT
    if (global_script.enable || !pwm_target_reached())
        return;
#else
    if (!pwm_target_reached())
        return;
#endif

    /* apply rgb and hsv offsets */
    pwm_modify_rgb(&msg->rgb, msg->step, msg->delay);
    pwm_modify_hsv(&msg->hsv, msg->step, msg->delay);
}

void parse_pull_int(struct remote_msg_pull_int_t *msg)
{
    /* pull pin to ground */
    remote_pull_int();

    uint8_t delay = msg->delay;

    /* start timer, delay between 50ms and 2550ms */
    if (delay == 0)
        delay = 5;
    else if (delay <= 51)
        delay *= 5;
    else
        delay = 255;

    timer_set(&remote.int_timer, delay);

    /* remember state */
    remote.int_state = INT_PULLED_TIMER;
}

void parse_config_startup(struct remote_msg_config_startup_t *msg)
{
    /* set mode and copy data */
    memcpy(&startup_config.params, &msg->params, sizeof(struct startup_parameters_t));
    /* save config to eeprom */
    storage_save_config();
}

void parse_bootloader(struct remote_msg_bootloader_t *msg)
{
    /* check magic bytes */
    if (msg->magic[0] != BOOTLOADER_MAGIC_BYTE1 ||
        msg->magic[1] != BOOTLOADER_MAGIC_BYTE2 ||
        msg->magic[2] != BOOTLOADER_MAGIC_BYTE3 ||
        msg->magic[3] != BOOTLOADER_MAGIC_BYTE4)
        return;

    /* wait until the tx fifo is empty, then start watchdog, but never kick it
     * (bootloader and firmware both disable the watchdog) */
    while (fifo_fill((fifo_t *)&global_uart.tx) > 0);
    wdt_enable(WDTO_120MS);
}

#endif
