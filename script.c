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
#include "config.h"
#include "script.h"
#include "pwm.h"

#if CONFIG_SCRIPT

/* global variables */
struct script_global_t script_global;

void script_init(void)
{
    /* initialize global structures */
    script_global.enable = 1;
    for (uint8_t i = 0; i < CONFIG_SCRIPT_TASKS; i++) {
        script_global.tasks[i].enable = 0;
        PT_INIT(&script_global.tasks[i].pt);
    }

    /* enable colorwheel program */
    struct colorwheel_params_t *params = &script_global.tasks[0].colorwheel;
    params->hue_start = 0;
    params->hue_step = 45;
    params->saturation = 255;
    params->value = 255;

    params->fade_step = 1;
    params->fade_delay = 2;

    script_global.tasks[0].execute = program_colorwheel;
    script_global.tasks[0].enable = 1;

    /* initialize timer, delay before start is 200ms */
    timer_set(&script_global.timer, 20);
}

void script_poll(void)
{
    if (!script_global.enable)
        return;

    if (timer_expired(&script_global.timer)) {
        for (uint8_t i = 0; i < CONFIG_SCRIPT_TASKS; i++) {
            struct process_t *task = &script_global.tasks[i];
            if (task->enable)
                task->execute(task);
        }

        /* recall after 100ms */
        timer_set(&script_global.timer, 10);
    }
}

#endif
