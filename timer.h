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

/* small timer library, uses timer2 */

#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* structures */
typedef struct {
    uint8_t current;
    uint8_t timeout;
} timer_t;

/* functions */
void timer_init(void);
void timer_set(timer_t *t, uint8_t timeout);
bool timer_expired(timer_t *t);

#endif
