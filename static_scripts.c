/* vim:fdm=marker ts=4 et ai
 * {{{
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
#include <stdint.h>
#include <avr/pgmspace.h>


/* local prototypes */

/* opcode handlers */
uint8_t opcode_handler_nop(uint8_t flags, uint8_t parameters[]);
uint8_t opcode_handler_fade_channel(uint8_t flags, uint8_t parameters[]);
uint8_t opcode_handler_stop(uint8_t flags, uint8_t parameters[]);


/* opcode lookup table */
uint8_t (*opcode_lookup_table[])(uint8_t flags, uint8_t parameters[]) = {
    &opcode_handler_nop,            /* opcode 0x00 */
    &opcode_handler_fade_channel,   /* opcode 0x10 */
    &opcode_handler_nop,            /* opcode 0x20 */
    &opcode_handler_nop,            /* opcode 0x30 */
    &opcode_handler_nop,            /* opcode 0x40 */
    &opcode_handler_nop,            /* opcode 0x50 */
    &opcode_handler_nop,            /* opcode 0x60 */
    &opcode_handler_nop,            /* opcode 0x70 */
    &opcode_handler_stop            /* opcode 0x80 */
        };



/* testscript */
static const uint8_t static_script_r[] PROGMEM = {
    MACRO_FADE_CHANNEL(CHANNEL_RED, 255, 0x400),
    MACRO_STOP()
                        };


uint16_t test_position = (uint16_t) &static_script_r;


/* init all structures in the global array 'script_threads' */
void init_script_threads(void)
/* {{{ */ {
    uint8_t i;

    /* iterate over all threads */
    for (i=0; i<MAX_THREADS; i++) {
        script_threads[i].handler.execute = 0;
        script_threads[i].handler.position = 0;

        script_threads[i].flags.channel_target_reached = 0;
        script_threads[i].flags.disabled = 1;

        script_threads[i].handler_stack_offset = 0;
    }

    script_threads[0].handler.execute = &memory_handler_flash;
    script_threads[0].handler.position = (uint16_t) &static_script_r;
    script_threads[0].flags.disabled = 0;
}
/* }}} */

/* iterate over all threads and execute each, if enabled */
void execute_script_threads(void)
/* {{{ */ {
    uint8_t i;

    UDR = 'T';
    while (!(UCSRA & (1<<UDRE)));

    /* iterate over all threads */
    for (i=0; i<MAX_THREADS; i++) {

        /* execute enabled threads */
        if (!script_threads[i].flags.disabled) {
            script_threads[i].handler.execute(&test_position);
        }
    }
}
/* }}} */

/* memory handlers */
void memory_handler_flash(uint16_t volatile *script_position)
/* {{{ */ {
    uint8_t opcode;
    uint8_t parameters[3];
    uint8_t i;

    UDR = 'H';
    while (!(UCSRA & (1<<UDRE)));
    UDR = HIGH(*script_position);
    while (!(UCSRA & (1<<UDRE)));
    UDR = LOW(*script_position);
    while (!(UCSRA & (1<<UDRE)));

    /* read opcode and parameters and call the appropiate opcode processing function */
    opcode = pgm_read_byte(*(script_position++));
    for (i=0; i<3; i++) {
        parameters[i] = pgm_read_byte(*(script_position++));
    }

    UDR = opcode;
    while (!(UCSRA & (1<<UDRE)));
    UDR = HIGH(*script_position);
    while (!(UCSRA & (1<<UDRE)));
    UDR = LOW(*script_position);
    while (!(UCSRA & (1<<UDRE)));

    /* call opcode handler */
    //(opcode_lookup_table[(opcode >> 4)])( (opcode & 0x0f), parameters);
}
/* }}} */

/* opcode handlers */
uint8_t opcode_handler_nop(uint8_t flags, uint8_t parameters[])
/* {{{ */ {

    /* do exactly nothing */
    (void) flags;
    (void) parameters;

    UDR = 'n';
    while (!(UCSRA & (1<<UDRE)));

    return 0;
}
/* }}} */

uint8_t opcode_handler_fade_channel(uint8_t flags, uint8_t parameters[])
/* {{{ */ {

    /* do exactly nothing */
    (void) flags;
    (void) parameters;

    UDR = 'f';
    while (!(UCSRA & (1<<UDRE)));

    return 0;
}
/* }}} */

uint8_t opcode_handler_stop(uint8_t flags, uint8_t parameters[])
/* {{{ */ {

    /* do exactly nothing */
    (void) flags;
    (void) parameters;

    UDR = 's';
    while (!(UCSRA & (1<<UDRE)));

    return 0;
}
/* }}} */


