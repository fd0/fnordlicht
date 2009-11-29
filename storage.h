/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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

#ifndef __STORAGE_H
#define __STORAGE_H

#include "color.h"
#include "remote-proto.h"
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdbool.h>

/* store a color configuration in EEPROM
 * size: 8 byte */
struct storage_color_t
{
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
    struct union_color_t color;
};

/* store the startup configuration in EEPROM
 * size: 12 byte */
struct storage_config_t
{
    enum startup_mode_t startup_mode;
    /* startup parameters, defined in remote_proto.h, size: 11 byte */
    union startup_parameters_t params;
};

/* storage structure for EEPROM
 * size: 494 byte
 * (means: 18 byte left) */
struct storage_t
{
    /* startup configuration, size: 12 byte */
    struct storage_config_t config;

    /* color storage, size: 60*8 == 480 byte */
    struct storage_color_t colors[CONFIG_EEPROM_COLORS];

    /* checksum, size: 2 byte */
    uint16_t checksum;
};

/* global structures */
extern struct storage_config_t startup_config;
extern EEMEM struct storage_t eeprom_storage;

/* initialize storage */
void storage_init(void);

/* save storage config cfg (or global_storage if cfg is NULL) to eeprom */
bool storage_save_config(void);
/* load storage config cfg (or global_storage if cfg is NULL) from eeprom */
bool storage_load_config(void);

void storage_save(uint8_t position, struct storage_color_t *color);
void storage_load(uint8_t position, struct storage_color_t *color);

/* return true if configuration has been loaded and is valid */
bool storage_valid(void);

#endif
