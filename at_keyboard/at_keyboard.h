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
