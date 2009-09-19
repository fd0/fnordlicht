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

#ifndef FIFO_H
#define FIFO_H

#include "config.h"

#include <stdint.h>
#include <avr/interrupt.h>

#ifndef UART_FIFO_SIZE
#error "UART_FIFO_SIZE is not defined"
#endif

#if (UART_FIFO_SIZE & (UART_FIFO_SIZE-1))
#error "UART_FIFO_SIZE is not a power of 2!"
#endif

/* structures */

/* capacity is UART_FIFO_SIZE-1 */
struct fifo_t {
    uint8_t buffer[UART_FIFO_SIZE];
    uint8_t front;
    uint8_t back;
    uint8_t size;
};

/* prototypes */
void fifo_init(volatile struct fifo_t *fifo, uint8_t fifo_size);
void fifo_store(volatile struct fifo_t *fifo, uint8_t data);
void fifo_store_buffer(volatile struct fifo_t *fifo, uint8_t data[]);
uint8_t fifo_load(volatile struct fifo_t *fifo);
uint8_t fifo_fill(volatile struct fifo_t *fifo);
uint8_t fifo_capacity(volatile struct fifo_t *fifo);

#endif
