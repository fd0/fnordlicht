/*
 * unzap firmware
 *
 * (c) by Alexander Neumann <alexander@lochraster.org>
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
 */

#ifndef __UI_H
#define __UI_H

#include <stdint.h>
#include <stdbool.h>

/* helper macros */
#define LED1_ON() LED_PORT |= _BV(LED1_PIN)
#define LED1_OFF() LED_PORT &= ~_BV(LED1_PIN)
#define LED1_TOGGLE() LED_PORT ^= _BV(LED1_PIN)
#define LED2_ON() LED_PORT |= _BV(LED2_PIN)
#define LED2_OFF() LED_PORT &= ~_BV(LED2_PIN)
#define LED2_TOGGLE() LED_PORT ^= _BV(LED2_PIN)

/* initialize the button and led pins */
void ui_init(void);

/* blink out a sequence (LSB first), every bit is 150ms long */
void ui_blink(uint8_t sequence1, uint8_t sequence2);

/* check if the current blink sequency is done */
bool ui_blinking(void);

/* poll for user actions */
void ui_poll(void);

#endif
