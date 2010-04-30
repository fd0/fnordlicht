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

#ifndef __IR_H
#define __IR_H

enum ir_mode_t
{
    IR_DISABLED    = 0,
    IR_RAW         = 1,    /* just receive a code, disable ir afterwards (-> DISABLE) */
    IR_DECODE      = 2,    /* just receive and decode vaules, disable ir afterwards (-> DISABLE) */
    IR_RECEIVE     = 3,    /* receive, decode and process codes */
};

struct ir_global_t
{
    enum ir_mode_t mode;
    uint16_t last;
    uint8_t length;

    #define MAX_CODE_LENGTH 160
    /* internal storage for received code: allocate 160*2 = 320 byte memory for
     * storing a code, this means we can store 80 on/off sequence timings */

    uint16_t time[MAX_CODE_LENGTH];
    uint8_t pos;
};

extern volatile struct ir_global_t ir_global;

void ir_init(void);
void ir_poll(void);

void ir_set_mode(enum ir_mode_t mode);

#define ir_disable() ir_set_mode(IR_DISABLED);
#define ir_enable() ir_set_mode(IR_RECEIVE);

#endif
