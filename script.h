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

#ifndef __SCRIPT_H
#define __SCRIPT_H

#include <stdint.h>
#include "config.h"
#include "pt/pt.h"
#include "timer.h"

#if !CONFIG_SCRIPT
#define script_init(...)
#define script_poll(...)

#else

/* structs */
struct process_t {
    PT_THREAD((*execute)(struct process_t *current));
    struct pt pt;
    uint8_t enable:1;
    uint16_t pos;
};

struct script_global_t {
    uint8_t enable:1;
    struct process_t tasks[CONFIG_SCRIPT_TASKS];
    timer_t timer;
};

/* global variables */
extern struct script_global_t script_global;

/* prototypes for scripting engine */
void script_init(void);
void script_poll(void);

/* prototypes for handler */
PT_THREAD(script_handler_fader_flash(struct process_t *process));
PT_THREAD(script_handler_wheel(struct process_t *process));

#endif
#endif
