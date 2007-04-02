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


#ifndef UART_H
#define UART_H

#include "config.h"

#if SERIAL_UART

#include "fifo.h"

/* define uart baud rate (19200) and mode (8N1) */
#if defined(__AVR_ATmega8__)
/* in atmega8, we need a special switching bit
 * for addressing UCSRC */
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)

#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__)
/* in atmega88, this isn't needed any more */
#define UART_UCSRC _BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0)
#endif

#define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)


/* structs */
struct global_uart_t {
    struct fifo_t rx_fifo;
    struct fifo_t tx_fifo;
};

/* global variables */
extern volatile struct global_uart_t global_uart;

/* prototypes */
void init_uart(void);
void uart_putc(uint8_t data);
void uart_puts(uint8_t buffer[]);

#endif

#endif
