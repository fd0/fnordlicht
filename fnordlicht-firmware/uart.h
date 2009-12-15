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

#ifndef UART_H
#define UART_H

#include "globals.h"
#include "fifo.h"
#include "../common/io.h"

#if !CONFIG_SERIAL

#define uart_init(...)
#define uart_putc(...)

#else

/* structs */
struct global_uart_t {
    fifo_t rx;
    fifo_t tx;
};

/* global variables */
extern volatile struct global_uart_t global_uart;

/* prototypes */
void uart_init(void);
void uart_putc(uint8_t data);

static inline bool uart_send_complete(void)
{
    return _BV(_UDRE_UART0) & _UCSRA_UART0;
}

#endif

#endif
