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

/* configure primary pwm pins (MUST be three successive pins in a port) */
#define PWM_PORT B
#define PWM_CHANNELS 3
#define PWM_CHANNEL_MASK (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define PWM_SHIFT 0

/* configure secondary pwm pins (MUST be three successive pins in a port) */
#define PWM2_PORT D
#define PWM2_CHANNEL_MASK (_BV(PD5) | _BV(PD6) | _BV(PD7))
#define PWM2_SHIFT 5

/* min brightness level */
#define PWM_MIN_BRIGHTNESS 10

/* configure maximal static program parameter size */
#define PROGRAM_PARAMETER_SIZE 10

/* number of color-configurations stored in EEPROM */
#define CONFIG_EEPROM_COLORS 60

/* define default supported number of tasks */
#define CONFIG_SCRIPT_TASKS 1

/* configure INT line for remote */
#define REMOTE_INT_PORT D
#define REMOTE_INT_PIN PD2

/* configure jumper pins for remote master mode */
#define REMOTE_MASTER_PORT C
#define REMOTE_MASTER_PIN1 PC2
#define REMOTE_MASTER_PIN2 PC3
/* configure master mode */
/* use 1 as address for the second device */
#define MASTER_MODE_FIRST_ADDRESS 1
/* wait 200ms before sending sync sequence */
#define MASTER_WAIT_BEFORE_SYNC 20
/* wait 15 min (900s) between program changes */
#define MASTER_MODE_SLEEP 900

/* configure storage bufer size */
#define STORAGE_BUFFER_SIZE 13

#endif /* _FNORDLICHT_CONFIG_H */
