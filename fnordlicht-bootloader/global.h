/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlichtmini serial bootloader
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

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "config.h"

/* configure primary pwm pins (MUST be three successive pins in a port) */
#define PWM_PORT B
#define PWM_CHANNELS 3
#define PWM_CHANNEL_MASK (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define PWM_SHIFT 0

/* configure INT pin */
#define REMOTE_INT_PORT D
#define REMOTE_INT_PIN PD2

/* abbreviations for port, ddr and pin */
#define P_PORT _OUTPORT(PWM_PORT)
#define P_DDR _DDRPORT(PWM_PORT)
#define R_PORT _OUTPORT(REMOTE_INT_PORT)
#define R_DDR _DDRPORT(REMOTE_INT_PORT)
#define R_PIN _INPORT(REMOTE_INT_PORT)
#define INTPIN REMOTE_INT_PIN

/* check if hardware is valid */
#if defined(HARDWARE_fnordlicht)
    /* specific settings for old fnordlicht hardware */
    #if !defined(PWM_INVERTED)
    #define PWM_INVERTED
    #endif
#elif defined(HARDWARE_fnordlichtmini)
    /* specific settings for fnordlichtmini hardware */

#else
#error "unknown HARDWARE platform!"
#endif

/* turn pwm output pins on and off */
#ifdef PWM_INVERTED
#define PWM_PIN_ON(bit) P_PORT &= ~_BV(bit) << (PWM_SHIFT)
#define PWM_PIN_OFF(bit) P_PORT |= _BV(bit) << (PWM_SHIFT)
#else
#define PWM_PIN_ON(bit) P_PORT |= _BV(bit) << (PWM_SHIFT)
#define PWM_PIN_OFF(bit) P_PORT &= ~_BV(bit) << (PWM_SHIFT)
#endif

#define PWM_PIN_TOGGLE(bit) P_PORT ^= _BV(bit) << (PWM_SHIFT)

#define PWM_RED 0
#define PWM_GREEN 1
#define PWM_BLUE 2

/* wait 100ms before checking INT pin (for delay_loop_2(50000)) */
#define CONFIG_DELAY F_CPU/4/50000/10
#define CONFIG_DELAY_LOOP2 50000

#endif
