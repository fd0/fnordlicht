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

#ifndef _FNORDLICHT_CONFIG_H
#define _FNORDLICHT_CONFIG_H

#include <avr/version.h>

/* check for avr-libc version */
#if __AVR_LIBC_VERSION__ < 10402UL
#error newer libc version (>= 1.4.2) needed!
#endif

/* check if cpu speed is defined */
#ifndef F_CPU
#error please define F_CPU!
#endif

/* check if this cpu is supported */
#if !(defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__))
#error "this cpu isn't supported yet!"
#endif

/* cpu specific configuration registers */
#if defined(__AVR_ATmega8__)
/* {{{ */
#define _ATMEGA8

#define _TIMSK_TIMER1 TIMSK
#define _UCSRB_UART0 UCSRB
#define _UDRIE_UART0 UDRIE
#define _TXEN_UART0 TXEN
#define _RXEN_UART0 RXEN
#define _RXCIE_UART0 RXCIE
#define _UBRRH_UART0 UBRRH
#define _UBRRL_UART0 UBRRL
#define _UCSRC_UART0 UCSRC
#define _UCSZ0_UART0 UCSZ0
#define _UCSZ1_UART0 UCSZ1
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

/* }}} */
#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__)
/* {{{ */
#define _ATMEGA88

#define _TIMSK_TIMER1 TIMSK1
#define _UCSRB_UART0 UCSR0B
#define _UDRIE_UART0 UDRIE0
#define _TXEN_UART0 TXEN0
#define _RXEN_UART0 RXEN0
#define _RXCIE_UART0 RXCIE0
#define _UBRRH_UART0 UBRR0H
#define _UBRRL_UART0 UBRR0L
#define _UCSRC_UART0 UCSR0C
#define _UCSZ0_UART0 UCSZ00
#define _UCSZ1_UART0 UCSZ01
#define _SIG_UART_RECV_UART0 SIG_USART_RECV
#define _SIG_UART_DATA_UART0 SIG_USART_DATA
#define _UDR_UART0 UDR0
/* }}} */
#endif


/* debug defines */
#ifndef DEBUG
#define DEBUG 0
#endif

/* include the script intpreter per default */
#ifndef STATIC_SCRIPTS
#define STATIC_SCRIPTS 1
#endif

/* include uart support per default */
#ifndef SERIAL_UART
#define SERIAL_UART 1
#endif

/* disable at keyboard decoder per default (EXPERIMENTAL) */
/* ATTENTION: THIS IS EXPERIMENTAL AND DOES NOT WORK ATM! */
#ifndef AT_KEYBOARD
#define AT_KEYBOARD 0
#endif

/* disable rc5-decoder per default */
#ifndef RC5_DECODER
#define RC5_DECODER 0
#endif

/* disable scripts speed control per default */
#ifndef SCRIPT_SPEED_CONTROL
#define SCRIPT_SPEED_CONTROL 0
#endif

/* disable i2c support per default (EXPERIMENTAL) */
/* ATTENTION: THIS IS EXPERIMENTAL AND DOES NOT WORK ATM! */
#ifndef I2C
#define I2C 0
#endif

#ifndef I2C_MASTER
#define I2C_MASTER 0
#endif

#ifndef I2C_SLAVE
#define I2C_SLAVE 0
#endif

/* fifo size must be a power of 2 and below 128 */
#define UART_FIFO_SIZE 32
#define UART_BAUDRATE 19200


/* color <-> channel assignment */
#define CHANNEL_RED     0
#define CHANNEL_GREEN   1
#define CHANNEL_BLUE    2

/* enable this if you want to control a fnordlicht via RS485 */
#ifndef RS485_CTRL
#define RS485_CTRL 0
#endif

#if (RS485_CTRL == 1) && (SERIAL_UART == 1)
#error "RS485_CTRL and SERIAL_UART are mutually exclusive!"
#endif


#endif /* _FNORDLICHT_CONFIG_H */
