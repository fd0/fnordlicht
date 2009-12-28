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

#ifndef _FNORDLICHT_GLOBALS_H
#define _FNORDLICHT_GLOBALS_H

#include <avr/version.h>
#include "config.h"
#include "../common/common.h"

/* check for avr-libc version */
#if __AVR_LIBC_VERSION__ < 10600UL
#error "newer libc version (>= 1.6.0) needed!"
#endif

/* check if cpu speed is defined */
#ifndef F_CPU
#error "please define F_CPU! (see Makefile)"
#endif

/* check if this cpu is supported */
#if !(defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__))
#error "this cpu is not supported yet!"
#endif

/* fifo size should be a power of 2 and below 128 */
#define CONFIG_FIFO_SIZE 64

/* serial communication is required */
#define CONFIG_SERIAL 1

/* leds */
#define LED_PORTNAME B
#define LED1_PIN     2
#define LED2_PIN     1

/* buttons */
#define BTN_PORTNAME C
#define BTN1_PIN     5
#define BTN2_PIN     4

/* convenient naming */
#define LED_PORT _OUTPORT(LED_PORTNAME)
#define LED_DDR  _DDRPORT(LED_PORTNAME)

#define BTN_PORT _OUTPORT(BTN_PORTNAME)
#define BTN_DDR  _DDRPORT(BTN_PORTNAME)
#define BTN_PIN  _INPORT(BTN_PORTNAME)

#endif /* _FNORDLICHT_CONFIG_H */
