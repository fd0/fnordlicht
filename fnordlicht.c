/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware next generation
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

#if SERIAL_UART
static inline void check_serial_input(uint8_t data);
#endif

#if SERIAL_UART
/** process serial data received by uart */
void check_serial_input(uint8_t data)
{

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
}
#endif

/** main function
 */
int main(void)
{
    pwm_init();

#if SERIAL_UART
    init_uart();
#endif

#if RC5_DECODER
    init_rc5();
#endif

    /* default color */
    global_pwm.channels[0].brightness = 50;
    global_pwm.channels[0].target_brightness = 50;

#if STATIC_SCRIPTS
    init_script_threads();

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

    /* enable interrupts globally */
    sei();

    while (1)
    {
        /* update pwm */
        pwm_poll();

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
    }
}
