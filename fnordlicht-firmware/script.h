/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
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

#ifndef __SCRIPT_H
#define __SCRIPT_H

#include <stdint.h>
#include <stdbool.h>
#include "globals.h"
#include "../common/pt/pt.h"
#include "timer.h"
#include "static_programs.h"

#if !CONFIG_SCRIPT
#define script_init(...)
#define script_poll(...)
#define script_stop(...)
#define script_start(...)
#define script_start_default(...)

#else

struct global_script_t {
    bool disable;
    struct process_t tasks[CONFIG_SCRIPT_TASKS];
    timer_t timer;
};

/* global variables */
extern struct global_script_t global_script;

/* prototypes for scripting engine */
void script_init(void);
void script_poll(void);
void script_stop(void);
void script_start(uint8_t task, uint8_t index, union program_params_t *params);
void script_start_default(void);

#endif
#endif
