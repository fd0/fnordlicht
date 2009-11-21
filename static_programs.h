/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef __STATIC_PROGRAMS_H
#define __STATIC_PROGRAMS_H

#include <stdint.h>
#include "config.h"
#include "pt/pt.h"

/* parameter structures (10 bytes) */
struct colorwheel_params_t {
    uint8_t fade_step;
    uint8_t fade_delay;
    uint16_t hue_start;
    int16_t hue_step;
    uint8_t saturation;
    uint8_t value;
};

/* global process struct */
struct process_t {
    PT_THREAD((*execute)(struct process_t *current));
    struct pt pt;
    uint8_t enable:1;
    union {
        /* parameters for static programs */
        uint8_t parameters[10];
        struct colorwheel_params_t colorwheel;
    };
};

#if CONFIG_SCRIPT

PT_THREAD(program_colorwheel(struct process_t *process));

#endif
#endif
