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

#include "globals.h"

#if CONFIG_SERIAL

#include "../common/io.h"
#include <avr/interrupt.h>

#include "globals.h"
#include "../common/common.h"
#include "pwm.h"
#include "fifo.h"
#include "uart.h"

/* define uart mode (8N1) */
#if defined(__AVR_ATmega8__)
/* in atmega8, we need a special switching bit
 * for addressing UCSRC */
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)

#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__)
/* in atmega88, this isn't needed any more */
#define UART_UCSRC _BV(_UCSZ0_UART0) | _BV(_UCSZ1_UART0)
#endif

/* global variables */
volatile struct global_uart_t global_uart;

/** output one character */
void uart_putc(uint8_t data)
{
    /* store data */
    fifo_enqueue((fifo_t *)&global_uart.tx, data);

    /* enable interrupt */
    _UCSRB_UART0 |= _BV(_UDRIE_UART0);
}

/** init the hardware uart */
void uart_init(void)
{
    #define BAUD CONFIG_SERIAL_BAUDRATE
    #include <util/setbaud.h>

    /* set baud rate */
    _UBRRH_UART0 = UBRRH_VALUE;
    _UBRRL_UART0 = UBRRL_VALUE;

    #if USE_2X
    _UCSRA_UART0 |= (1 << _U2X_UART0);
    #endif

    /* set mode */
    _UCSRC_UART0 = UART_UCSRC;

    /* enable transmitter, receiver and receiver complete interrupt */
    _UCSRB_UART0 = _BV(_TXEN_UART0) | _BV(_RXEN_UART0) | _BV(_RXCIE_UART0);

    /* init fifos */
    fifo_init((fifo_t *)&global_uart.rx);
    fifo_init((fifo_t *)&global_uart.tx);
}


/** interrupts*/

/** uart receive interrupt */
ISR(_SIG_UART_RECV_UART0)
{

    /* store received data */
    fifo_enqueue((fifo_t *)&global_uart.rx, _UDR_UART0);

}

/** uart data register empty interrupt */
ISR(_SIG_UART_DATA_UART0)
{

    /* load next byte to transfer */
    _UDR_UART0 = fifo_dequeue((fifo_t *)&global_uart.tx);

    /* check if this interrupt is still needed */
    if ( fifo_fill((fifo_t *)&global_uart.tx) == 0) {
        /* disable this interrupt */
        _UCSRB_UART0 &= ~_BV(_UDRIE_UART0);
    }

}


#endif
