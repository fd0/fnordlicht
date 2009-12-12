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

#ifndef IO_H
#define IO_H

#include <avr/io.h>

/* cpu specific configuration registers */
#if defined(__AVR_ATmega8__)

#define _TIMSK_TIMER1 TIMSK
#define _TIFR_TIMER1 TIFR
#define _UCSRB_UART0 UCSRB
#define _UCSRA_UART0 UCSRA
#define _UDRIE_UART0 UDRIE
#define _UDRE_UART0 UDRE
#define _RXC_UART0 RXC
#define _TXC_UART0 TXC
#define _TXEN_UART0 TXEN
#define _RXEN_UART0 RXEN
#define _RXCIE_UART0 RXCIE
#define _UBRRH_UART0 UBRRH
#define _UBRRL_UART0 UBRRL
#define _UCSRC_UART0 UCSRC
#define _UCSZ0_UART0 UCSZ0
#define _UCSZ1_UART0 UCSZ1
#define _U2X_UART0 U2X
#define _SIG_UART_RECV_UART0 SIG_UART_RECV
#define _SIG_UART_DATA_UART0 SIG_UART_DATA
#define _UDR_UART0 UDR
#define UCSR0A UCSRA
#define UCSR0C UCSRC
#define MPCM0 MPCM
#define UCSZ00 UCSZ0
#define UCSZ01 UCSZ1
#define UCSZ02 UCSZ2
#define UBRR0H UBRRH
#define UBRR0L UBRRL
#define UCSR0B UCSRB
#define RXEN0 RXEN
#define TXEN0 TXEN
#define RXC0 RXC
#define RXB80 RXB8
#define UDR0 UDR
#define _IVREG GICR
#ifndef MCUSR
#define MCUSR MCUCSR
#endif

#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__)

#define _TIMSK_TIMER1 TIMSK1
#define _TIFR_TIMER1 TIFR1
#define _UCSRA_UART0 UCSR0A
#define _UCSRB_UART0 UCSR0B
#define _UDRIE_UART0 UDRIE0
#define _UDRE_UART0 UDRE0
#define _RXC_UART0 RXC0
#define _TXC_UART0 TXC0
#define _TXEN_UART0 TXEN0
#define _RXEN_UART0 RXEN0
#define _RXCIE_UART0 RXCIE0
#define _UBRRH_UART0 UBRR0H
#define _UBRRL_UART0 UBRR0L
#define _UCSRC_UART0 UCSR0C
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
#define _U2X_UART0 U2X0
#define _SIG_UART_RECV_UART0 SIG_USART_RECV
#define _SIG_UART_DATA_UART0 SIG_USART_DATA
#define _UDR_UART0 UDR0
#define _IVREG MCUCR
#endif


#endif
