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

#include <avr/pgmspace.h>
#include "static_programs.h"
#include "color.h"
#include "pwm.h"
#include "timer.h"

#if CONFIG_SCRIPT

/* global list of programs */
PROGMEM program_handler static_programs[] = {
    program_colorwheel,
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

#endif
