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

/* includes */
#include "config.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "common.h"
#include "fnordlicht.h"
#include "pwm.h"
#include "uart.h"
#include "i2c.h"

#if RC5_DECODER
#include "rc5.h"
#endif


#if STATIC_SCRIPTS

/* include static scripts */
#include "static_scripts.h"
#include "testscript.h"

#endif

/* structs */
volatile struct global_t global = {{0, 0}};

/* prototypes */
void (*jump_to_bootloader)(void) = (void *)0xc00;
static inline void init_output(void);

#if SERIAL_UART
static inline void check_serial_input(uint8_t data);
#endif

/** init output channels */
void init_output(void) { /* {{{ */
    /* set all channels high -> leds off */
    PORTB = _BV(PB0) | _BV(PB1) | _BV(PB2);
    /* configure PB0-PB2 as outputs */
    DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);
}

/* }}} */

#if SERIAL_UART
/** process serial data received by uart */
void check_serial_input(uint8_t data)
/* {{{ */ {

    switch (data) {
#if 0
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
#if SCRIPT_SPEED_CONTROL
        case '>':
            script_threads[0].speed_adjustment--;
            script_threads[1].speed_adjustment--;
            script_threads[2].speed_adjustment--;
            break;
        case '<':
            script_threads[0].speed_adjustment++;
            script_threads[1].speed_adjustment++;
            script_threads[2].speed_adjustment++;
            break;
#endif
        case 'i':
            TWCR |= _BV(TWSTA) | _BV(TWINT);
            break;
#endif
        case 'p':
            jump_to_bootloader();
            break;
    }
} /* }}} */
#endif

/** main function
 */
int main(void) {
    init_output();
    init_pwm();

#if SERIAL_UART
    init_uart();
#endif

#if RC5_DECODER
    init_rc5();
#endif

#if I2C
    init_i2c();
#endif

    global_pwm.channels[0].brightness = 50;
    global_pwm.channels[0].target_brightness = 50;

#if STATIC_SCRIPTS
    init_script_threads();

    #if RS485_CTRL == 0
    /* start the example scripts */
    script_threads[0].handler.execute = &memory_handler_flash;
    script_threads[0].handler.position = (uint16_t) &colorchange_red;
    script_threads[0].flags.disabled = 0;

    //script_threads[1].handler.execute = &memory_handler_flash;
    //script_threads[1].handler.position = (uint16_t) &testscript_flash2;
    //script_threads[1].flags.disabled = 0;
    //
    //script_threads[2].handler.execute = &memory_handler_eeprom;
    //script_threads[2].handler.position = (uint16_t) &testscript_eeprom;
    //script_threads[2].flags.disabled = 0;

    //script_threads[0].handler.execute = &memory_handler_flash;
    //script_threads[0].handler.position = (uint16_t) &blinken;
    //script_threads[0].flags.disabled = 0;
    #endif

#endif

#if I2C_MASTER
    i2c_global.send_messages[0].command.size = 4;
    i2c_global.send_messages[0].command.code = COMMAND_SET_COLOR;
    i2c_global.send_messages[0].command.set_color_parameters.colors[0] = 0x10;
    i2c_global.send_messages[0].command.set_color_parameters.colors[1] = 0x10;
    i2c_global.send_messages[0].command.set_color_parameters.colors[2] = 0x10;

    i2c_global.send_messages_count = 1;
#endif

#if RS485_CTRL
    /* init command bus */
    UCSR0A = _BV(MPCM0); /* enable multi-processor communication mode */
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); /* 9 bit frame size */

    #define UART_UBRR 8 /* 115200 baud at 16mhz */
    UBRR0H = HIGH(UART_UBRR);
    UBRR0L = LOW(UART_UBRR);

    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(UCSZ02); /* enable receiver and transmitter */
#endif

    /* enable interrupts globally */
    sei();

    while (1) {
        /* after the last pwm timeslot, rebuild the timeslot table */
        if (global.flags.last_pulse) {
            global.flags.last_pulse = 0;

            update_pwm_timeslots();
        }

        /* at the beginning of each pwm cycle, call the fading engine and
         * execute all script threads */
        if (global.flags.new_cycle) {
            global.flags.new_cycle = 0;

            update_brightness();
#if STATIC_SCRIPTS
            execute_script_threads();
#endif

            continue;
        }


#if SERIAL_UART
        /* check if we received something via uart */
        if (fifo_fill(&global_uart.rx_fifo) > 0) {
            check_serial_input(fifo_load(&global_uart.rx_fifo));
            continue;
        }
#endif


#if RC5_DECODER
        /* check if we received something via ir */
        if (global_rc5.new_data) {
            static uint8_t toggle_bit = 2;

            /* if key has been pressed again */
            if (global_rc5.received_command.toggle_bit != toggle_bit) {

                /* if code is 0x01 (key '1' on a default remote) */
                if (global_rc5.received_command.code == 0x01) {

                    /* install script into thread 1 */
                    script_threads[1].handler.execute = &memory_handler_flash;
                    script_threads[1].handler.position = (uint16_t) &green_flash;
                    script_threads[1].flags.disabled = 0;
                    script_threads[1].handler_stack_offset = 0;

                }

                /* store new toggle bit state */
                toggle_bit = global_rc5.received_command.toggle_bit;

            }

            /* reset the new_data flag, so that new commands can be received */
            global_rc5.new_data = 0;

            continue;
        }
#endif

#if RS485_CTRL
        if (UCSR0A & _BV(RXC0)) {

            uint8_t address = UCSR0B & _BV(RXB80); /* read nineth bit, zero if data, one if address */
            uint8_t data = UDR0;
            static uint8_t buffer[8];
            static uint8_t fill = 0;

            if (UCSR0A & _BV(MPCM0) || address) { /* if MPCM mode is still active, or ninth bit set, this is an address packet */

                /* check if we are ment */
                if (data == 0 || data == RS485_ADDRESS) {

                    /* remove MPCM flag and reset buffer fill counter */
                    UCSR0A &= ~_BV(MPCM0);
                    fill = 0;

                    continue;

                } else {/* turn on MPCM */

                    UCSR0A |= _BV(MPCM0);
                    continue;

                }
            }

            /* else this is a data packet, put data into buffer */
            buffer[fill++] = data;

            if (buffer[0] == 0x01) {  /* soft reset */

                jump_to_bootloader();

            } else if (buffer[0] == 0x02 && fill == 4) { /* set color */

                for (uint8_t pos = 0; pos < 3; pos++) {
                    global_pwm.channels[pos].target_brightness = buffer[pos + 1];
                    global_pwm.channels[pos].brightness = buffer[pos + 1];
                }

                UCSR0A |= _BV(MPCM0); /* return to MPCM mode */

            } else if (buffer[0] == 0x03 && fill == 6) { /* fade to color */

                for (uint8_t pos = 0; pos < 3; pos++) {
                    global_pwm.channels[pos].speed_h = buffer[1];
                    global_pwm.channels[pos].speed_l = buffer[2];
                    global_pwm.channels[pos].target_brightness = buffer[pos + 3];
                }

                UCSR0A |= _BV(MPCM0); /* return to MPCM mode */
            }

        }
#endif

#if I2C_MASTER
        i2c_master_check_queue();
#endif
    }
}
