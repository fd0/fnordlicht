/* vim:fdm=marker ts=4 et ai
 * {{{
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include "../config.h"

#ifndef AT_KEYBOARD_H
#define AT_KEYBOARD_H

#if AT_KEYBOARD

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/parity.h>

#define PS2_CLOCK PD3
#define PS2_DATA PD4

/* structs */
union key_t {
    uint16_t bits;
    struct {
        uint8_t scancode:8;
        uint8_t parity_bit:1;
        uint8_t stop_bit:1;
    };
};

struct keyboard_flags {
    uint8_t receiption_active:1;
    uint8_t transmission_active:1;
    uint8_t transmission_start:1;
    uint8_t transmission_ack:1;
    uint8_t receive_complete:1;
    union {
        uint8_t received_scancode;
        uint8_t transmit_scancode;
    };
};

/* global variables */
volatile struct keyboard_flags global_keyboard;

/* prototypes */
void init_at_keyboard(void);
void send_byte_to_keyboard(uint8_t data);

#endif

#endif
