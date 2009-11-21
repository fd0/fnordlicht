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
#include <string.h>
#include "config.h"
#include "script.h"
#include "static_programs.h"
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
    union program_params_t params;
    params.colorwheel.hue_start = 0;
    params.colorwheel.hue_step = 60;
    params.colorwheel.saturation = 255;
    params.colorwheel.value = 255;

    params.colorwheel.fade_step = 1;
    params.colorwheel.fade_delay = 2;
    params.colorwheel.fade_sleep = 0;

    script_start(0, 0, &params);

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

void script_stop(void)
{
    /* stop all tasks */
    for (uint8_t i = 0; i < CONFIG_SCRIPT_TASKS; i++) {
        script_global.tasks[i].enable = 0;
        PT_INIT(&script_global.tasks[i].pt);
    }
}

void script_start(uint8_t task, uint8_t index, union program_params_t *params)
{
    /* check for valid index */
    if (index >= STATIC_PROGRAMS_LEN)
        return;

    /* copy params from pointer to task structure */
    memcpy(&script_global.tasks[task].params, params, sizeof(struct process_t));

    /* load program handler */
    script_global.tasks[task].execute = (program_handler)pgm_read_word(&static_programs[index]);

    /* enable script */
    script_global.tasks[task].enable = 1;
}

#endif
