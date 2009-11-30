/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
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

#include <stdlib.h>
#include <avr/pgmspace.h>
#include "static_programs.h"
#include "color.h"
#include "pwm.h"
#include "timer.h"
#include "remote.h"
#include "common.h"
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
    static uint8_t sleep;

    PT_BEGIN(&process->pt);

    static struct hsv_color_t c;

    c.hue = process->params.colorwheel.hue_start;
    c.value = process->params.colorwheel.value;
    c.saturation = process->params.colorwheel.saturation;

    while (1) {
        /* set new color */
        pwm_fade_hsv(&c, process->params.colorwheel.fade_step, process->params.colorwheel.fade_delay);
        c.hue += process->params.colorwheel.hue_step;

        /* wait until target reached */
        PT_WAIT_UNTIL(&process->pt, pwm_target_reached());

        /* sleep (remember: we are called every 100ms) */
        if (process->params.colorwheel.fade_sleep > 0) {
            sleep = process->params.colorwheel.fade_sleep;

            while (sleep--)
                PT_YIELD(&process->pt);
        }
    }

    PT_END(&process->pt);
}

PT_THREAD(program_random(struct process_t *process))
{
    PT_BEGIN(&process->pt);

    /* initialize random generator */
    uint16_t seed = process->params.random.seed;
    if (process->params.random.use_address) {
        uint16_t address = remote_address();
        seed ^= address;
    }
    srandom(process->params.random.seed);

    static struct hsv_color_t c;
    c.value = process->params.random.value;
    c.saturation = process->params.random.saturation;

    while (1) {
        /* generate new color */
        union uint32_t_access rnd;
        rnd.raw = random();

        /* use lower word for hue */
        rnd.words[0] %= 360;

        /* check for minimal color distance, regenerate random if not */
        if (process->params.random.min_distance) {
            if (c.hue > rnd.words[0] && c.hue - rnd.words[0] < process->params.random.min_distance)
                continue;
            if (c.hue < rnd.words[0] && rnd.words[0] - c.hue < process->params.random.min_distance)
                continue;
        }

        /* copy color to structure */
        c.hue = rnd.words[0];

        /* fade to new color */
        pwm_fade_hsv(&c, process->params.random.fade_step, process->params.random.fade_delay);

        /* wait until target reached */
        PT_WAIT_UNTIL(&process->pt, pwm_target_reached());

        /* sleep (remember: we are called every 100ms) */
        static uint8_t sleep;

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
    PT_BEGIN(&process->pt);

    static uint8_t pos;
    static enum {
        UP = 0,
        DOWN = 1,
    } direction;
    static struct storage_color_t c;

    /* reset variables */
    pos = process->params.replay.start;
    direction = UP;

    while (1) {
        /* load next color value */
        storage_load(pos, &c);

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

            pwm_fade_hsv(&color, c.step, c.delay);
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
