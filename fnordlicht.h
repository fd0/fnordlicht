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

#ifndef __FNORDLICHT_H
#define __FNORDLICHT_H

/* global flag(=bit) structure */
struct flags_t {
    /* set by pwm interrupt after burst, signals the beginning of a new pwm
     * cycle to the main loop. */
    uint8_t pwm_start:1;
    /* set by pwm interrupt after last interrupt in the current cycle, signals
     * the main loop to rebuild the pwm timslot table */
    uint8_t pwm_last_pulse:1;
};

struct global_t {
    struct flags_t flags;
};

extern volatile struct global_t global;

#endif
