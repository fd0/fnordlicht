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

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

/* macros for extracting low and high byte */
#define LO8(x) (uint8_t)(0x00ff & (x))
#define HI8(x) (uint8_t)((0xff00 & (x)) >> 8)

/* macros for concatenating PORT, PIN and DDR */
#define _CONCAT(a, b)            a ## b
#define _OUTPORT(name)           _CONCAT(PORT, name)
#define _INPORT(name)            _CONCAT(PIN, name)
#define _DDRPORT(name)           _CONCAT(DDR, name)

/* __noinline attribute (opposite of inline attribute */
#define __noinline __attribute__((noinline))

/* structure for accessing bytes and words in an uint32_t */
union uint32_t_access {
    uint8_t bytes[4];
    uint16_t words[2];
    uint32_t raw;
};

#endif
