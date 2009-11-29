/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
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

#ifndef _FNORDLICHT_CONFIG_H
#define _FNORDLICHT_CONFIG_H

#include <avr/version.h>

/* check for avr-libc version */
#if __AVR_LIBC_VERSION__ < 10600UL
#error "newer libc version (>= 1.6.0) needed!"
#endif

/* check if cpu speed is defined */
#ifndef F_CPU
#error "please define F_CPU! (see Makefile)"
#endif

/* check if this cpu is supported */
#if !(defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__))
#error "this cpu isn't supported yet!"
#endif

/* cpu specific configuration registers */
#if defined(__AVR_ATmega8__)
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

#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__)
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
#endif


/* debug defines */
#ifndef CONFIG_DEBUG
#define CONFIG_DEBUG 0
#endif

/* include the script interpreter per default */
#ifndef CONFIG_SCRIPT
#define CONFIG_SCRIPT 1
#endif

/* define default supported number of tasks */
#ifndef CONFIG_SCRIPT_TASKS
#define CONFIG_SCRIPT_TASKS 1
#endif

/* define default startup script (0 is colorwheel, 1 is random) */
#ifndef CONFIG_SCRIPT_DEFAULT
#define CONFIG_SCRIPT_DEFAULT 0
#endif

/* include uart support per default */
#ifndef CONFIG_SERIAL
#define CONFIG_SERIAL 1
#endif

/* include remote command support per default */
#ifndef CONFIG_REMOTE
#define CONFIG_REMOTE 1
#endif

/* set default baudrate */
#define CONFIG_SERIAL_BAUDRATE 19200

/* fifo size should be a power of 2 and below 128 */
#define CONFIG_FIFO_SIZE 64

/* number of pwm channels */
#define PWM_CHANNELS 3
#define PWM_CHANNEL_MASK (_BV(PB0) | _BV(PB1) | _BV(PB2))
#define PWM_DDR DDRB
#define PWM_PORT PORTB

/* color <-> channel assignment */
#define CHANNEL_RED     0
#define CHANNEL_GREEN   1
#define CHANNEL_BLUE    2

/* configure normal or inverted pwm */
#if !defined(PWM_INVERTED) && defined(HARDWARE_fnordlicht)
#define PWM_INVERTED
#endif

/* configure maximal static program parameter size */
#define PROGRAM_PARAMETER_SIZE 10

/* number of color-configurations stored in EEPROM */
#define CONFIG_EEPROM_COLORS 60

#endif /* _FNORDLICHT_CONFIG_H */
