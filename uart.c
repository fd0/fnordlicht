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

#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "pwm.h"
#include "uart.h"

/* define uart baud rate (19200) and mode (8N1) */
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)
#define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)

/** init the hardware uart */
void init_uart(void)
/*{{{*/ {
    /* set baud rate */
    UBRRH = (uint8_t)(UART_UBRR >> 8);  /* high byte */
    UBRRL = (uint8_t)UART_UBRR;         /* low byte */

    /* set mode */
    UCSRC = UART_UCSRC;

    /* enable transmitter, receiver and receiver complete interrupt */
    UCSRB = _BV(TXEN) | _BV(RXEN) | _BV(RXCIE);

    /* send boot message */
    UDR = 'B';

    /* wait until boot message has been sent over the wire */
    while (!(UCSRA & (1<<UDRE)));
} /* }}} */

/** interrupts*/

/** uart receive interrupt */
SIGNAL(SIG_UART_RECV)
/*{{{*/ {
    uint8_t data = UDR;

    switch (data) {
        case '1':
            global_pwm.channels[0].target_brightness-=1;
            break;
        case '4':
            global_pwm.channels[0].target_brightness+=1;
            break;
        case '2':
            global_pwm.channels[1].target_brightness-=1;
            break;
        case '5':
            global_pwm.channels[1].target_brightness+=1;
            break;
        case '3':
            global_pwm.channels[2].target_brightness-=1;
            break;
        case '6':
            global_pwm.channels[2].target_brightness+=1;
            break;
        case '0':
            global_pwm.channels[0].target_brightness=0;
            global_pwm.channels[1].target_brightness=0;
            global_pwm.channels[2].target_brightness=0;
            break;
        case '=':
            global_pwm.channels[0].target_brightness=global_pwm.channels[0].brightness;
            global_pwm.channels[1].target_brightness=global_pwm.channels[1].brightness;
            global_pwm.channels[2].target_brightness=global_pwm.channels[2].brightness;
            break;
        case '>':
            global_pwm.channels[0].speed >>= 1;
            global_pwm.channels[1].speed >>= 1;
            global_pwm.channels[2].speed >>= 1;
            break;
        case '<':
            global_pwm.channels[0].speed <<= 1;
            global_pwm.channels[1].speed <<= 1;
            global_pwm.channels[2].speed <<= 1;
            break;
        case 's':
            UDR = HIGH(global_pwm.channels[0].speed);
            while (!(UCSRA & _BV(UDRE)));
            UDR = LOW(global_pwm.channels[0].speed);
            while (!(UCSRA & _BV(UDRE)));
            break;
        case 'b':
            UDR = global_pwm.channels[0].brightness;
            while (!(UCSRA & _BV(UDRE)));
            UDR = global_pwm.channels[1].brightness;
            while (!(UCSRA & _BV(UDRE)));
            UDR = global_pwm.channels[2].brightness;
            while (!(UCSRA & _BV(UDRE)));
            break;
    }
} /*}}}*/
