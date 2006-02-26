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

#if I2C

#include <avr/io.h>
#include <avr/interrupt.h>

#include "common.h"
#include "i2c.h"


/* global variables */
volatile struct i2c_global_t i2c_global;


void init_i2c(void) {
    /* init global structure */
#if I2C_SLAVE
    i2c_global.received_messages_count = 0;
#endif

#if I2C_MASTER
    i2c_global.send_messages_count = 0;
    i2c_global.transmission_active = 0;
#endif

    /* load baudrate (lowest baudrate possible) */
    TWBR = 0xff;

    /* no prescaler */
    TWSR = 0;

    /* load own i2c address and general call enable bit */
    TWAR = (I2C_ADDRESS << 1) | (I2C_GENERAL_CALL);

    /* enable twi, clear old twi state, enable auto-ack, enable twi interrupt */
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE) | _BV(TWEA);
}

#if I2C_MASTER
void i2c_master_check_queue(void)
/* {{{ */ {
    /* if we have something to send and no transmission is active, initiate transmission */
    //if (i2c_global.send_messages_count > 0 && !i2c_global.transmission_active) {
    //    //uart_putc('M');
    //    TWCR |= _BV(TWSTA) | _BV(TWINT);
    //}
}
/* }}} */
#endif


/* twi interrupt */
ISR(SIG_2WIRE_SERIAL) {
    uint8_t state;

    /* mask prescaler bits (least significant 3 bits) */
    state = TWSR & 0xf8;

    uart_putc('I');
    uart_putc(state);

    i2c_global.transmission_active = 1;
}

#endif
