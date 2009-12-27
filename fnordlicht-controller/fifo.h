/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
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

#ifndef __FIFO_H
#define __FIFO_H

#include <stdint.h>
#include <stdbool.h>
#include "globals.h"

typedef uint8_t fifo_content_t;
typedef uint8_t fifo_size_t;

typedef struct
{
    fifo_size_t read;
    fifo_size_t write;
    fifo_content_t buffer[CONFIG_FIFO_SIZE];
} fifo_t;

void fifo_init(fifo_t *f);
void fifo_enqueue(fifo_t *f, fifo_content_t data);
fifo_content_t fifo_dequeue(fifo_t *f);
fifo_size_t fifo_fill(fifo_t *f);
bool fifo_empty(fifo_t *f);
bool fifo_full(fifo_t *f);

#endif
