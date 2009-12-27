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
#include "usb.h"
#include "timer.h"
#include "uart.h"

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
    usb_init();
    timer_init();
    uart_init();

    /* configure leds */
    DDRB |= _BV(PB1) | _BV(PB2);

    /* enable interrupts globally */
    sei();

    usb_enable();

    timer_t tmr;

    timer_set(&tmr, 50);

    while (1)
    {
        usb_poll();

        if (timer_expired(&tmr)) {
            PINB = _BV(PB2);
            timer_set(&tmr, 50);
        }
    }
}
