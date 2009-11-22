/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlicht
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
#include "config.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "common.h"
#include "fnordlicht.h"
#include "pwm.h"
#include "uart.h"
#include "remote.h"
#include "timer.h"
#include "script.h"

/* structs */
volatile struct global_t global = {{0}};

/** main function
 */
int main(void)
{
    pwm_init();
    timer_init();
    uart_init();
    script_init();
    script_start_default();

#if !CONFIG_SCRIPT
    /* default color */
    global_pwm.target.red = 50;
#endif

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
    }
}
