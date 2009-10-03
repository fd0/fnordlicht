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

    /* enable static fading script */
    script_global.tasks[0].execute = script_handler_fader_flash;
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


#define FADER_FLASH_ENTRYSIZE (PWM_CHANNELS + 2)
#define FADER_FLASH_ENTRIES (sizeof(fader_flash)/FADER_FLASH_ENTRYSIZE)
PROGMEM uint8_t fader_flash[] = {
/*    R    G    B    step,  delay */
    255,   0,   0,      1,      1,
    255, 255,   0,      1,      1,
    0,   255,   0,      1,      1,
    0,   255, 255,      1,      1,
    0,     0, 255,      1,      1,
    255,   0, 255,      1,      1,
};

PT_THREAD(script_handler_fader_flash(struct process_t *process))
{
    PT_BEGIN(&process->pt);

    while (1) {
        /* load new fader values */
        uint8_t step = pgm_read_byte_near(&fader_flash[process->pos + PWM_CHANNELS]);
        uint8_t delay = pgm_read_byte_near(&fader_flash[process->pos + PWM_CHANNELS + 1]);

        /* load new color */
        for (uint8_t c = 0; c < PWM_CHANNELS; c++) {
            global_pwm.target.rgb[c] = pgm_read_byte_near(&fader_flash[process->pos + c]);
            global_pwm.fade_step[c] = step;
            global_pwm.fade_delay[c] = delay;
        }

        process->pos += FADER_FLASH_ENTRYSIZE;
        if (process->pos == sizeof(fader_flash))
            process->pos = 0;

        /* wait until target reached */
        for (uint8_t c = 0; c < PWM_CHANNELS; c++)
            PT_WAIT_UNTIL(&process->pt,
                    global_pwm.current.rgb[c] == global_pwm.target.rgb[c]);
    }

    PT_END(&process->pt);
}

#endif
