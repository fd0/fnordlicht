/*
 *   ox - infrared to usb keyboard/mouse adapter
 *
 *   by Alexander Neumann <alexander@lochraster.org>
 *
 * inspired by InfraHID by Alex Badea,
 * see http://vamposdecampos.googlepages.com/infrahid.html
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
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <util/crc16.h>
#include "config.h"
#include "globals.h"
#include "../common/common.h"
#include "usb.h"
#include "ir.h"
#include "ir-cluster.h"
#include "ui.h"

/* macro magic */
#define IR_DDR      _DDRPORT(IR_PORTNAME)
#define IR_PORT     _OUTPORT(IR_PORTNAME)

#if IR_INTNUM < 8
#   define IR_BANK      0
#   define IR_VECT      PCINT0_vect
#elif IR_INTNUM < 16
#   define IR_BANK      1
#   define IR_VECT      PCINT1_vect
#else
#   define IR_BANK      2
#   define IR_VECT      PCINT2_vect
#endif

#define IR_INT          (IR_INTNUM - IR_BANK * 8)
#define IR_PCIE         _PCIE(IR_BANK)
#define IR_PCIF         _PCIF(IR_BANK)
#define IR_PCMSK        _PCMSK(IR_BANK)

/* 8 should be enough for everybody! */
#define MAX_CLUSTERS 8

/* internal state machine */
static enum {
    READY = 0,
    LISTEN = 1,
    DONE = 2,
} state;

volatile struct ir_global_t ir_global;

void ir_init(void)
{
    /* initialize input pin (with pullup) */
    IR_DDR &= ~_BV(IR_PIN);
    IR_PORT |= _BV(IR_PIN);

    /* configure timer1 with prescaler 64 and CTC for measuring ir timings */
    TCCR1B = _BV(CS11) | _BV(CS10) | _BV(WGM12);
    /* configure timer action after 20ms: 20mhz/64/50 */
    OCR1A = F_CPU/50/64;
}

void ir_set_mode(enum ir_mode_t mode)
{
    if (ir_global.mode != IR_DISABLED && mode == IR_DISABLED) {
        /* disable pin change interrupt */
        IR_PCMSK &= ~_BV(IR_INT);
        PCICR &= ~_BV(IR_PCIE);

        /* clear interrupt flags */
        TIFR1 = TIFR1;
    } else if (ir_global.mode == IR_DISABLED && mode != IR_DISABLED) {
        /* clear interrupt flags */
        TIFR1 = TIFR1;

        /* reset variables */
        ir_global.pos = 0;
        state = READY;

        /* enable pin change interrupt */
        IR_PCMSK |= _BV(IR_INT);
        PCICR |= _BV(IR_PCIE);
    }

    ir_global.mode = mode;

    if (mode == IR_DECODE || mode == IR_RAW)
        ui_blink(0, 0x5);
}

ISR(IR_VECT, ISR_NOBLOCK)
{
    /* store code */
    uint16_t value = TCNT1;
    TCNT1 = 0;

    /* do not store the first 'off' timing value */
    if (state == READY)
        state = LISTEN;
    else if (state == LISTEN) {
        ir_global.time[ir_global.pos++] = value;

        /* check if we reached the end of the array */
        if (ir_global.pos == MAX_CODE_LENGTH)
            state = DONE;
    }
}

void ir_poll(void)
{
    if (ir_global.mode == IR_DISABLED)
        return;

    /* check for timeout */
    if (TIFR1 & _BV(OCF1A)) {
        /* clear interrupt flag */
        TIFR1 = _BV(OCF1A);

        /* if ir has been detected, process code, else return to listening */
        if (ir_global.pos > 1) {
            state = DONE;

            /* set last timing to zero */
            if (ir_global.pos % 2 == 0)
                ir_global.time[ir_global.pos-1] = 0;
            else
                ir_global.time[ir_global.pos++] = 0;
        } else
            state = READY;
    }

    /* if code is complete, process! */
    if (state == DONE) {
        /* compute clusters */
        if (ir_global.mode == IR_RECEIVE || ir_global.mode == IR_DECODE) {
            uint16_t cluster_on[MAX_CLUSTERS];
            uint16_t cluster_off[MAX_CLUSTERS];

            uint8_t con = ir_cluster(&ir_global.time[0], ir_global.pos/2, &cluster_on[0], MAX_CLUSTERS);
            uint8_t coff = ir_cluster(&ir_global.time[1], ir_global.pos/2-1, &cluster_off[0], MAX_CLUSTERS);

            for (uint8_t i = 0; i < ir_global.pos/2; i++) {
                uint8_t data_on = ir_min_cluster(ir_global.time[2*i], cluster_on, con);
                uint8_t data_off = ir_min_cluster(ir_global.time[2*i+1], cluster_off, coff);

                ir_global.time[i] = data_on | (data_off << 8);
            }

            ir_global.pos /= 2;
        }

        if (ir_global.mode == IR_RECEIVE) {
            uint16_t crc = 0;
            for (uint8_t i = 0; i < ir_global.pos; i++) {
                crc = _crc16_update(crc, LO8(ir_global.time[i]));
                crc = _crc16_update(crc, HI8(ir_global.time[i]));
            }

            /* remember code and length */
            ir_global.last = crc;
            ir_global.length = ir_global.pos/2;

            /*
             *  insert code evaluation here...
             */

            ui_blink(0x1, 0);

            ir_global.pos = 0;
            state = READY;
        } else {
            ui_blink(0, 0x1);
            ir_set_mode(IR_DISABLED);
        }
    }
}
