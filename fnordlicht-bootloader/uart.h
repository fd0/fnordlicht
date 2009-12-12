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

#include <stdbool.h>

/* prototypes */
void uart_init(void);
void uart_putc(uint8_t data);

static inline bool uart_receive_complete(void)
{
    return _UCSRA_UART0 & _BV(_RXC_UART0);
}


static inline bool uart_send_complete(void)
{
    return _BV(_UDRE_UART0) & _UCSRA_UART0;
}

static inline uint8_t uart_getc(void)
{
    return _UDR_UART0;
}

#endif
