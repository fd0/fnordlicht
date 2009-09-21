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

#if STATIC_SCRIPTS
/* include static scripts */
#include "static_scripts.h"
#include "testscript.h"
#endif

/* structs */
volatile struct global_t global = {{0}};

/** main function
 */
int main(void)
{
    pwm_init();
    timer_init();

#if SERIAL_UART
    init_uart();
#endif

    /* default color */
    global_pwm.channels[0].target_brightness = 50;

#if STATIC_SCRIPTS
    init_script_threads();

    /* start the example script */
    script_threads[0].handler.execute = &memory_handler_flash;
    script_threads[0].handler.position = (uint16_t) &colorchange_red;
    script_threads[0].flags.disabled = 0;
#endif

    /* enable interrupts globally */
    sei();

    while (1)
    {
        /* update pwm */
        pwm_poll();

#if SERIAL_UART
        /* check for remote commands */
        remote_poll();
#endif
    }
}
