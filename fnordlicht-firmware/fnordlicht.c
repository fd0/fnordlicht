/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
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

/* includes */
#include "globals.h"
#include "../common/io.h"

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include "../common/common.h"
#include "pwm.h"
#include "uart.h"
#include "remote.h"
#include "timer.h"
#include "script.h"
#include "storage.h"

static void startup(void)
{
    /* if configuration is valid */
    if (storage_valid_config()) {

        /* read default mode from storage (do nothing if mode is invalid) */
        if (startup_config.params.mode == STARTUP_PROGRAM) {
            /* start program */
            script_start(0, startup_config.params.program, (union program_params_t *)startup_config.params.program_parameters);
        }
    } else {
        /* start default program */
        script_start_default();

#if !CONFIG_SCRIPT
        /* or set some default color */
        global_pwm.target.red = 50;
#endif
    }
}

/* NEVER CALL DIRECTLY! */
void disable_watchdog(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void disable_watchdog(void)
{
    MCUSR = 0;
    wdt_disable();
}

/** main function
 */
int main(void)
{
    pwm_init();
    timer_init();
    uart_init();
    storage_init();
    remote_init();
    script_init();

    /* do high-level startup configuration */
    startup();

    /* enable interrupts globally */
    sei();

    while (1)
    {
        /* update pwm */
        pwm_poll();

        /* check for remote commands */
        remote_poll();

        /* update pwm */
        pwm_poll();

        /* call scripting */
        script_poll();

        /* update pwm */
        pwm_poll();

        /* update fading */
        pwm_poll_fading();

        /* update pwm */
        pwm_poll();

        /* process storage requests */
        storage_poll();
    }
}
