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
#include <avr/pgmspace.h>
#include "static_programs.h"
#include "color.h"
#include "pwm.h"
#include "timer.h"
#include "remote.h"
#include "../common/common.h"
#include "storage.h"

#if CONFIG_SCRIPT

/* global list of programs */
PROGMEM program_handler static_programs[] = {
    program_colorwheel,
    program_random,
    program_replay,
};

PT_THREAD(program_colorwheel(struct process_t *process))
{
    static uint16_t sleep;
    static struct hsv_color_t c, c2;

    PT_BEGIN(&process->pt);

    c.hue = process->params.colorwheel.hue_start;
    c.value = process->params.colorwheel.value;
    c.saturation = process->params.colorwheel.saturation;

    uint8_t add = process->params.colorwheel.add_addr;
    c.hue += remote_address() * add * process->params.colorwheel.hue_step;

    while (1) {
        /* set new color */
        memcpy(&c2, &c, sizeof(c));
        /* apply offsets */
        apply_hsv_offset(&c2);
        uint8_t step = apply_offset(
                process->params.colorwheel.fade_step,
                global_remote.offsets.step);
        uint8_t delay = apply_offset(
                process->params.colorwheel.fade_delay,
                global_remote.offsets.delay);
        pwm_fade_hsv(&c2, step, delay);

        c.hue += process->params.colorwheel.hue_step;

        /* wait until target reached */
        PT_WAIT_UNTIL(&process->pt, pwm_target_reached());

        /* sleep (in seconds, remember: we are called every 100ms) */
        sleep = 10 * process->params.colorwheel.fade_sleep;
        while (sleep--)
            PT_YIELD(&process->pt);
    }

    PT_END(&process->pt);
}

PT_THREAD(program_random(struct process_t *process))
{
    static uint16_t sleep;
    static struct hsv_color_t c;

    PT_BEGIN(&process->pt);

    /* initialize random generator */
    uint16_t seed = process->params.random.seed;
    if (process->params.random.use_address)
        seed ^= remote_address();
    srandom(seed);

    while (1) {
        /* generate new color */
        union uint32_t_access rnd;
        rnd.raw = random();

        /* use lower word for hue */
        rnd.words[0] %= 360;

        /* check for minimal color distance, regenerate random if not */
        int16_t min_distance = process->params.random.min_distance;
        if (min_distance) {
            int16_t distance = c.hue - rnd.words[0];
            if (distance < 0)
                distance = -distance;

            if (distance > 180)
                distance = 360-distance;

            if (distance < min_distance)
                continue;
        }

        /* copy color to structure, reset saturation and value */
        c.hue = rnd.words[0];
        c.value = process->params.random.value;
        c.saturation = process->params.random.saturation;

        /* apply offsets */
        apply_hsv_offset(&c);
        uint8_t step = apply_offset(
                process->params.random.fade_step,
                global_remote.offsets.step);
        uint8_t delay = apply_offset(
                process->params.random.fade_delay,
                global_remote.offsets.delay);
        /* fade to new color */
        pwm_fade_hsv(&c, step, delay);

        /* wait until target reached */
        if (process->params.random.wait_for_fade)
            PT_WAIT_UNTIL(&process->pt, pwm_target_reached());

        /* sleep (remember: we are called every 100ms) */
        if (process->params.random.fade_sleep > 0) {
            sleep = process->params.random.fade_sleep;

            while (sleep--)
                PT_YIELD(&process->pt);
        }
    }

    PT_END(&process->pt);
}

PT_THREAD(program_replay(struct process_t *process))
{
    static uint8_t pos;
    static enum {
        UP = 0,
        DOWN = 1,
    } direction;
    static struct storage_color_t c;

    PT_BEGIN(&process->pt);

    /* reset variables */
    pos = process->params.replay.start;
    direction = UP;

    while (1) {
        /* load next color value */
        storage_load_color(pos, &c);

        if (c.color.rgb_marker == 0xff) {
            struct rgb_color_t color;
            color.red = c.color.red;
            color.green = c.color.green;
            color.blue = c.color.blue;

            pwm_fade_rgb(&color, c.step, c.delay);
        } else {
            struct hsv_color_t color;
            color.hue = c.color.hue;
            color.saturation = c.color.saturation;
            color.value = c.color.value;

            /* apply offsets */
            apply_hsv_offset(&color);
            uint8_t step = apply_offset(
                    process->params.random.fade_step,
                    global_remote.offsets.step);
            uint8_t delay = apply_offset(
                    process->params.random.fade_delay,
                    global_remote.offsets.delay);
            pwm_fade_hsv(&color, step, delay);
        }

        /* update pos */
        if (direction == UP) {
            /* check for upper end */
            if (pos == process->params.replay.end) {
                switch (process->params.replay.repeat) {
                    case REPEAT_NONE:       PT_EXIT(&process->pt);
                                            break;
                    case REPEAT_START:      pos = process->params.replay.start;
                                            break;
                    case REPEAT_REVERSE:    direction = DOWN;
                                            pos--;
                                            break;
                }
            } else
                pos++;
        } else {
            if (pos == process->params.replay.start) {
                direction = UP;
                pos++;
            } else
                pos--;
        }

        /* wait */
        while (c.pause--)
            PT_YIELD(&process->pt);
    }

    PT_END(&process->pt);
}

#endif
