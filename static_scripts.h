/*
 vim:fdm=marker ts=4 et ai
 */

/* {{{
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


#ifndef fnordlicht_thread_h
#define fnordlicht_thread_h

#include "config.h"
#include "common.h"
#include <avr/io.h>
#include <stdint.h>

#define MAX_THREAD_STACK_DEPTH 3
#define MAX_THREADS 3

/* structs */

struct script_handler_t { /* {{{ */
    uint8_t (*execute)(uint8_t thread_table_offset);
    uint16_t position;
}; /* }}} */

struct thread_t { /* {{{ */
    struct script_handler_t handler;

    struct {
        uint8_t channel_target_reached:PWM_CHANNELS;    /* these channels reached their target brightness value lately */
        uint8_t disabled;                               /* disable execution of this thread */
    } flags;

    struct script_handler_t stack[MAX_THREAD_STACK_DEPTH];
    uint8_t stack_offset;
}; /* }}} */

/* global variables */
struct thread_t script_threads[MAX_THREADS];

/* prototypes */
void init_script_threads(void);
void execute_script_threads(void);

#endif
