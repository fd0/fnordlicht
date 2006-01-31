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


/* define the cpu speed when using the fuse bits set by the
 * makefile (and documented in fuses.txt), using an external
 * crystal oscillator at 16MHz */

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* debug defines */
#ifndef DEBUG
#define DEBUG 0
#endif

/* include the script intpreter per default */
#ifndef STATIC_SCRIPTS
#define STATIC_SCRIPTS 1
#endif

/* fifo size must be a power of 2 and below 128 */
#define UART_FIFO_SIZE 32
#define UART_BAUDRATE 19200


/* color <-> channel assignment */
#define CHANNEL_RED     0
#define CHANNEL_GREEN   1
#define CHANNEL_BLUE    2
