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

#include "static_scripts.h"

/* init all structures in the global array 'script_threads' */
inline void init_script_threads(void) { /* {{{ */
    uint8_t i;

    /* iterate over all threads */
    for (i=0; i<MAX_THREADS; i++) {
        script_threads[i].handler.execute = 0;
        script_threads[i].handler.position = 0;

        script_threads[i].flags.channel_target_reached = 0;
        script_threads[i].flags.disabled = 1;

        script_threads[i].stack_offset = 0;
    }
}

/* }}} */

/* iterate over all threads and execute each, if enabled */
inline void execute_script_threads(void) { /* {{{ */
    uint8_t i;

    /* iterate over all threads */
    for (i=0; i<MAX_THREADS; i++) {

        /* execute enabled threads */
        if (!script_threads[i].flags.disabled) {
            script_threads[i].handler.execute(i);
        }
    }
}

/* }}} */

