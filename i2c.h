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

#ifndef I2C_H
#define I2C_H

#include "config.h"

#if I2C

#include "uart.h"


/* defines */
/* {{{ */
#define I2C_BUFFER_SIZE 8

#if (I2C_BUFFER_SIZE & (I2C_BUFFER_SIZE-1))
#warning I2C_BUFFER_SIZE is not a power of 2!
#endif

#ifndef I2C_ADDRESS
#define I2C_ADDRESS 0x02    /* 7 bit! */
#endif

#ifndef I2C_GENERAL_CALL
#define I2C_GENERAL_CALL 1  /* enable general call */
#endif

/* }}} */

/* commands */
/* {{{ */
#define COMMAND_SET_COLOR 0
/* }}} */


/* structs */

/* encapsulates a remote command, received somehow */
struct remote_command_t
/* {{{ */ {
    union {
        uint8_t raw[5];

        struct {
            uint8_t size;   /* the size of this command, excluding this byte */
            uint8_t code;   /* opcode */
            union {         /* contains one struct for every possible command */

                struct {    /* set color */
                    uint8_t colors[3];  /* contains values for red, green and blue */
                } set_color_parameters;

            };
        };
    };
};
/* }}} */

/* HAL: an i2c_message structure contains one remote_command */
struct i2c_message_t
/* {{{ */ {
    uint8_t destination_address;        /* destination i2c address */
    struct remote_command_t command;    /* the received command */
};
/* }}} */


struct i2c_global_t
/* {{{ */ {
#if I2C_SLAVE
    uint8_t received_messages_count;
    struct remote_command_t received_messages[I2C_BUFFER_SIZE-1];
#endif

#if I2C_MASTER
    uint8_t send_messages_count;
    struct i2c_message_t send_messages[I2C_BUFFER_SIZE-1];

    uint8_t transmission_active;
#endif
};
/* }}} */

/* global variables */
extern volatile struct i2c_global_t i2c_global;

/* prototypes */
void init_i2c(void);

#if I2C_MASTER
void i2c_master_check_queue(void);
#endif

#endif

#endif
