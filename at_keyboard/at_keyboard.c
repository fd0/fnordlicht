/* vim:fdm=marker ts=4 et ai
 * {{{
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#include "../config.h"

#if AT_KEYBOARD

#include "at_keyboard.h"

#define F_CPU 16000000UL

#include <util/delay.h>

/** init int1 */
void init_at_keyboard(void)
/*{{{*/ {
    /* configure pins as inputs and enable pullups */
    DDRD &= ~ (_BV(PS2_DATA) | _BV(PS2_CLOCK));
    PORTD |= _BV(PS2_DATA) | _BV(PS2_CLOCK);

    /* interrupt at falling edge */
    MCUCR |= _BV(ISC11);
    MCUCR &= ~ _BV(ISC10);

    /* enable int1 interrupt */
    GICR |= _BV(INT1);

    /* clear flags */
    global_keyboard.receiption_active = 0;
    global_keyboard.receive_complete = 0;
    global_keyboard.received_scancode = 0;
} /* }}} */

/** send byte to keyboard */
void send_byte_to_keyboard(uint8_t data)
/*{{{*/ {
    /* wait until all data has been received */
    //while (global_keyboard.receiption_active == 1)
    //    asm volatile ("nop");

    /* disable external interrupt */
    GICR &= ~_BV(INT1);

    /* next external interrupt should occure at the falling edge */
    MCUCR &= ~_BV(ISC10);

    /* set flag and store data */
    global_keyboard.transmission_active = 1;
    global_keyboard.transmission_start = 1;
    global_keyboard.transmit_scancode = data;

    /* configure clock and data line as output */
    DDRD |= _BV(PS2_CLOCK) | _BV(PS2_DATA);

    /* take clock line low */
    PORTD &= ~_BV(PS2_CLOCK);

    /* take data line low */
    PORTD &= ~_BV(PS2_DATA);

    /* take clock line high and reconfigure clock line as input */
    PORTD |= _BV(PS2_CLOCK);
    DDRD &= ~_BV(PS2_CLOCK);

    /* no we wait for the keyboard to generate the clock... */

    /* reenable external interrupt */
    GICR |= _BV(INT1);

} /* }}} */

/** external interrupt 1 */
SIGNAL(SIG_INTERRUPT1)
/*{{{*/ {
    static int8_t bitcounter;
    static union key_t key;

    /* if ISC10 is not set, this is a falling edge */
    if (! (MCUCR & _BV(ISC10))) {

        /* transmission mode */
        if (global_keyboard.transmission_active) {
            /* check if this is the first falling edge */
            if (global_keyboard.transmission_start) {
                global_keyboard.transmission_start = 0;
                global_keyboard.transmission_ack = 0;
                bitcounter = 0;
            }

            /* or the last falling edge */
            if (bitcounter == 11) {
                /* test if the keyboard ack'ed this data word, line must be low */
                if (! (PIND & _BV(PS2_DATA))) {
                    global_keyboard.transmission_ack = 1;
                } else {
                    global_keyboard.transmission_ack = 0;
                }
            }

        /* receiption mode */
        } else {
            /* if not already in receiption, check if this is the start bit */
            if (!global_keyboard.receiption_active) {

                /* if DATA is low, this is the start bit */
                if ((PIND & PS2_DATA)) {
                    key.bits = 0;
                    global_keyboard.receiption_active = 1;
                    bitcounter = -1;
                }

                /* receiption is active */
            } else {

                /* load data */
                if (PIND & _BV(PS2_DATA)) {
                    key.bits |= (1<<(bitcounter));
                } else
                    key.bits &= ~(1<<(bitcounter));
            }

            PORTB = bitcounter;
        }

    /* else this is a rising edge */
    } else {
        /* transmission mode */
        if (global_keyboard.transmission_active) {

            /* first 8 bits are data bits */
            if (bitcounter < 8) {
                if (global_keyboard.transmit_scancode & (1<<bitcounter)) {
                    /* send one */
                    PORTD |= _BV(PS2_DATA);
                } else {
                    /* send zero */
                    PORTD &= ~_BV(PS2_DATA);
                }
            /* next bit is parity */
            } else if (bitcounter == 8) {
                /* transmit odd bit parity (== !even parity) */
                if ( parity_even_bit(global_keyboard.transmit_scancode) == 0 ) {
                    /* send one */
                    PORTD |= _BV(PS2_DATA);
                } else {
                    /* send zero */
                    PORTD &= ~_BV(PS2_DATA);
                }
            /* next bit is stop-bit (always one) */
            } else if (bitcounter == 9) {

                /* output bit, reconfigure as input */
                PORTD |= _BV(PS2_DATA);
                DDRD &= ~_BV(PS2_DATA);
            /* last rising edge */
            } else if (bitcounter == 11) {
                if (global_keyboard.transmission_ack) {
                    UDR = 'A';
                } else {
                    UDR = 'a';
                }

                /* reset transmission active flag */
                global_keyboard.transmission_active = 0;

                bitcounter = 0;
            }

            /* increase bitcounter */
            bitcounter++;

        /* receiption mode */
        } else {
            if (global_keyboard.receiption_active) {
                /* update bitcounter */
                bitcounter++;

                /* check if we are done */
                if (bitcounter == 10) {

                    global_keyboard.receiption_active = 0;

                    /* test stop bit and parity */
                    if ((!parity_even_bit(key.scancode)) == key.parity_bit && key.stop_bit == 1) {
                        global_keyboard.receive_complete = 1;
                        global_keyboard.received_scancode = key.scancode;
                    }
                }

                //UDR = bitcounter;
            }
        }
    }

    /* toggle bit to interrupt at the other edge next time */
    MCUCR ^= _BV(ISC10);
} /*}}}*/

#endif
