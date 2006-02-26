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


#include "config.h"

#if SERIAL_UART

#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "pwm.h"
#include "fifo.h"
#include "uart.h"

/* global variables */
volatile struct global_uart_t global_uart;

/** output one character */
inline void uart_putc(uint8_t data)
/*{{{*/ {
    /* store data */
    fifo_store(&global_uart.tx_fifo, data);

    /* enable interrupt */
    _UCSRB_UART0 |= _BV(_UDRIE_UART0);
} /* }}} */

/** output a string */
inline void uart_puts(uint8_t buffer[])
/*{{{*/ {
    /* store data */
    fifo_store_buffer(&global_uart.tx_fifo, buffer);

    /* enable interrupt */
    _UCSRB_UART0 |= _BV(_UDRIE_UART0);
} /* }}} */

/** init the hardware uart */
void init_uart(void)
/*{{{*/ {
    /* set baud rate */
    _UBRRH_UART0 = (uint8_t)(UART_UBRR >> 8);  /* high byte */
    _UBRRL_UART0 = (uint8_t)UART_UBRR;         /* low byte */

    /* set mode */
    _UCSRC_UART0 = UART_UCSRC;

    /* enable transmitter, receiver and receiver complete interrupt */
    _UCSRB_UART0 = _BV(_TXEN_UART0) | _BV(_RXEN_UART0) | _BV(_RXCIE_UART0);

    /* init fifos */
    fifo_init(&global_uart.rx_fifo, UART_FIFO_SIZE-1);
    fifo_init(&global_uart.tx_fifo, UART_FIFO_SIZE-1);

    /* send boot message */
    uart_putc('B');
} /* }}} */


/** interrupts*/

/** uart receive interrupt */
ISR(_SIG_UART_RECV_UART0)
/*{{{*/ {

    /* store received data */
    fifo_store(&global_uart.rx_fifo, _UDR_UART0);

} /*}}}*/

/** uart data register empty interrupt */
ISR(_SIG_UART_DATA_UART0)
/*{{{*/ {

    /* load next byte to transfer */
    _UDR_UART0 = fifo_load(&global_uart.tx_fifo);

    /* check if this interrupt is still needed */
    if ( fifo_fill(&global_uart.tx_fifo) == 0) {
        /* disable this interrupt */
        _UCSRB_UART0 &= ~_BV(_UDRIE_UART0);
    }

} /*}}}*/


#endif
