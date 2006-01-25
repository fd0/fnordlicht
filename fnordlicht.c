/* vim:fdm=marker ts=4 et ai
 * {{{
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
 *     Lars Noschinski <lars@public.noschinski.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 }}} */

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
#include "static_scripts.h"

/* static scripts */
#include "testscript.h"

/* structs */
volatile struct global_t global = {{0, 0}};

/* prototypes */
static inline void init_output(void);

/** init output channels */
void init_output(void) { /* {{{ */
    /* set all channels high -> leds off */
    PORTB = _BV(PB0) | _BV(PB1) | _BV(PB2);
    /* configure PB0-PB2 as outputs */
    DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);
}

/* }}} */

/** main function
 */
int main(void) {
    init_output();
    init_uart();
    init_timer1();
    init_pwm();
    init_script_threads();

    /* start the example scripts */
    script_threads[0].handler.execute = &memory_handler_flash;
    script_threads[0].handler.position = (uint16_t) &testscript_flash1;
    script_threads[0].flags.disabled = 0;

    script_threads[1].handler.execute = &memory_handler_flash;
    script_threads[1].handler.position = (uint16_t) &testscript_flash2;
    script_threads[1].flags.disabled = 0;

    script_threads[2].handler.execute = &memory_handler_eeprom;
    script_threads[2].handler.position = (uint16_t) &testscript_eeprom;
    script_threads[2].flags.disabled = 0;

    /* enable interrupts globally */
    sei();

    while (1) {
        /* at the beginning of each pwm cycle, call the fading engine and
         * execute all script threads */
        if (global.flags.new_cycle) {
            global.flags.new_cycle = 0;

            update_brightness();
            execute_script_threads();
        }

        if (global.flags.last_pulse) {
            global.flags.last_pulse = 0;

            update_pwm_timeslots();
        }
    }
}
