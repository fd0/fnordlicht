/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
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
#include "remote.h"
#include "../common/pt/pt.h"
#include <avr/eeprom.h>
#include <util/crc16.h>
#include <string.h>

/* global structures */
struct storage_config_t startup_config;
EEMEM struct storage_t eeprom_storage;

/* internal state */
struct storage_internal_t
{
    bool eeprom_good;

    /* buffer for nonblocking read/write */
    uint8_t buf[STORAGE_BUFFER_SIZE];
    uint8_t index;
    uint8_t *address;
    uint8_t bytes_remaining;
    struct pt thread;

    enum {
        STORAGE_IDLE = 0,
        STORAGE_WRITE = 1,
    } state;
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
    storage_load_config();

#ifdef INIT_ZERO
    /* initialize state structure */
    storage.state = STORAGE_IDLE;
    PT_INIT(&storage.thread);
#endif
}

static PT_THREAD(storage_thread(struct pt *thread))
{
    PT_BEGIN(thread);

    while (1) {
        if (storage.state == STORAGE_WRITE) {
            /* pull int line */
            remote_pull_int();

            /* write data */
            while (storage.bytes_remaining--) {
                eeprom_write_byte(storage.address++, storage.buf[storage.index++]);

                while (!eeprom_is_ready())
                    PT_YIELD(thread);
            }

            /* update checksum */
            eeprom_write_word(&eeprom_storage.checksum, eeprom_checksum());
            while (!eeprom_is_ready())
                PT_YIELD(thread);

            /* release int line */
            remote_release_int();

            storage.state = STORAGE_IDLE;
        }

        PT_YIELD(thread);
    }

    PT_END(thread);
}

void storage_poll(void)
{
    /* call storage thread */
    if (storage.state != STORAGE_IDLE)
        PT_SCHEDULE(storage_thread(&storage.thread));
}

void storage_save_config(void)
{
    if (storage.state != STORAGE_IDLE)
        return;

    /* set magic byte and save startup_config to EEPROM */
    startup_config.magic = EEPROM_MAGIC_BYTE;

    /* copy data to buffer */
    memcpy(&storage.buf[0], &startup_config, sizeof(struct storage_config_t));
    /* set address and bytes_remaining */
    storage.index = 0;
    storage.bytes_remaining = sizeof(struct storage_config_t);
    storage.address = (uint8_t *)&eeprom_storage.config;
    /* start write */
    storage.state = STORAGE_WRITE;

    /* reset magic config and mark EEPROM as good */
    startup_config.magic = 0;
    storage.eeprom_good = true;
}

void storage_load_config(void)
{
    if (!storage.eeprom_good)
        return;

    /* load config */
    eeprom_read_block(&startup_config, &eeprom_storage.config, sizeof(struct storage_config_t));
}

void storage_save_color(uint8_t position, struct storage_color_t *color)
{
    if (storage.state != STORAGE_IDLE)
        return;

    /* copy data to buffer */
    memcpy(&storage.buf[0], color, sizeof(struct storage_color_t));
    /* set address and bytes_remaining */
    storage.index = 0;
    storage.bytes_remaining = sizeof(struct storage_color_t);
    storage.address = (uint8_t *)&eeprom_storage.color[position];
    /* start write */
    storage.state = STORAGE_WRITE;
}

void storage_load_color(uint8_t position, struct storage_color_t *color)
{
    eeprom_read_block(color, &eeprom_storage.color[position], sizeof(struct storage_color_t));
}

bool storage_valid_config(void)
{
    return (storage.eeprom_good && startup_config.magic == EEPROM_MAGIC_BYTE);
}
