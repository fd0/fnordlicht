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

#include "storage.h"
#include "pt/pt.h"
#include <avr/eeprom.h>
#include <util/crc16.h>

/* global structures */
struct storage_config_t startup_config;
EEMEM struct storage_t eeprom_storage;

/* internal state */
struct storage_internal_t
{
    bool eeprom_good;
};

static struct storage_internal_t storage;

static uint16_t eeprom_checksum(void)
{
    uint16_t checksum = 0;
    uint8_t *start = NULL;

    for (uint16_t i = 0; i < sizeof(struct storage_t)-2; i++)
        checksum = _crc16_update(checksum, eeprom_read_byte(start++));

    return checksum;
}

void storage_init(void)
{
    /* compare checksum and eeprom configuration content */
    uint16_t checksum1 = eeprom_checksum();
    uint16_t checksum2 = eeprom_read_word(&eeprom_storage.checksum);
    storage.eeprom_good = (checksum1 == checksum2);
    if (storage.eeprom_good)
        storage_load_config();
}

bool storage_save_config(void)
{
    eeprom_write_block(&startup_config, &eeprom_storage.config, sizeof(struct storage_config_t));
    eeprom_write_word(&eeprom_storage.checksum, eeprom_checksum());
    storage.eeprom_good = true;
    return true;
}

bool storage_load_config(void)
{
    if (!storage.eeprom_good)
        return false;

    /* load config */
    eeprom_read_block(&startup_config, &eeprom_storage.config, sizeof(struct storage_config_t));

    return true;
}

void storage_save_color(uint8_t position, struct storage_color_t *color)
{
    eeprom_write_block(color, &eeprom_storage.color[position], sizeof(struct storage_color_t));
    while(!eeprom_is_ready());
    eeprom_write_word(&eeprom_storage.checksum, eeprom_checksum());
    while(!eeprom_is_ready());
}

void storage_load_color(uint8_t position, struct storage_color_t *color)
{
    eeprom_read_block(color, &eeprom_storage.color[position], sizeof(struct storage_color_t));
}

bool storage_valid(void)
{
    return storage.eeprom_good;
}
