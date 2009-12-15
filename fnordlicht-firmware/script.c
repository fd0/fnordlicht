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

#include <avr/pgmspace.h>
#include <string.h>
#include "globals.h"
#include "script.h"
#include "static_programs.h"
#include "pwm.h"

#if CONFIG_SCRIPT

/* global variables */
struct global_script_t global_script;

void script_init(void)
{
    /* initialize global structures */
#ifdef INIT_ZERO
    for (uint8_t i = 0; i < CONFIG_SCRIPT_TASKS; i++) {
        global_script.tasks[i].enable = 0;
        PT_INIT(&global_script.tasks[i].pt);
    }
#endif

    /* initialize timer, delay before start is 200ms */
    timer_set(&global_script.timer, 20);
}

void script_start_default(void)
{
#ifdef CONFIG_SCRIPT_DEFAULT
#if CONFIG_SCRIPT_DEFAULT == 0
    /* enable colorwheel program */
    union program_params_t params;
    params.colorwheel.hue_start = 0;
    params.colorwheel.hue_step = 60;
    params.colorwheel.add_addr = 0;
    params.colorwheel.saturation = 255;
    params.colorwheel.value = 255;

    params.colorwheel.fade_step = 1;
    params.colorwheel.fade_delay = 2;
    params.colorwheel.fade_sleep = 0;

    script_start(0, 0, &params);
#elif CONFIG_SCRIPT_DEFAULT == 1
    /* enable colorwheel program */
    union program_params_t params;
    params.random.seed = 23;
    params.random.use_address = 0;
    params.random.wait_for_fade = 1;
    params.random.min_distance = 60;

    params.random.saturation = 255;
    params.random.value = 255;

    params.random.fade_step = 1;
    params.random.fade_delay = 3;
    params.random.fade_sleep = 100;

    script_start(0, 1, &params);
#else
#warning "CONFIG_SCRIPT_DEFAULT has unknown value!  No default program is started."
#endif
#endif
}

void script_poll(void)
{
    if (global_script.disable)
        return;

    if (timer_expired(&global_script.timer)) {
        for (uint8_t i = 0; i < CONFIG_SCRIPT_TASKS; i++) {
            struct process_t *task = &global_script.tasks[i];
            if (task->enable) {
                /* run task, disable if exited */
                if (PT_SCHEDULE(task->execute(task)) == 0)
                    task->enable = false;
            }
        }

        /* recall after 100ms */
        timer_set(&global_script.timer, 10);
    }
}

void script_stop(void)
{
    /* stop all tasks */
    for (uint8_t i = 0; i < CONFIG_SCRIPT_TASKS; i++) {
        global_script.tasks[i].enable = false;
        PT_INIT(&global_script.tasks[i].pt);
    }

    /* disable global */
    global_script.disable = true;
}

void script_start(uint8_t task, uint8_t index, union program_params_t *params)
{
    /* check for valid index */
    if (index >= STATIC_PROGRAMS_LEN)
        return;

    /* enable global */
    global_script.disable = false;

    /* copy params from pointer to task structure */
    memcpy(&global_script.tasks[task].params, params, sizeof(union program_params_t));

    /* load program handler */
    global_script.tasks[task].execute = (program_handler)pgm_read_word(&static_programs[index]);

    /* enable script */
    global_script.tasks[task].enable = true;

    /* reset timer */
    timer_set(&global_script.timer, 10);
}

#endif
