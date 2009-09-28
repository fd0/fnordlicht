/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware next generation
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

#ifndef __REMOTE_PROTO
#define __REMOTE_PROTO 1

#define REMOTE_MSGLEN 10

#define REMOTE_CMD_SET 0x01
#define REMOTE_CMD_FADE 0x02
#define REMOTE_CMD_CONFIG 0x03
#define REMOTE_CMD_RESYNC 0x1b

/* bootloader commands */
#define REMOTE_CMD_BOOT_CONFIG 0x80
#define REMOTE_CMD_BOOT_FLASH 0x81

#define REMOTE_ADDR_BROADCAST 0xff

struct remote_msg_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t data[REMOTE_MSGLEN-2];
};

struct remote_msg_fade
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    uint8_t target[PWM_CHANNELS];
};

struct remote_msg_config
{
    uint8_t address;
    uint8_t cmd;
    uint8_t scripting;
};

enum mem_type_t {
    MEM_FLASH = 0,
    MEM_EEPROM = 1,
};

struct remote_msg_boot_config
{
    uint8_t address;
    uint8_t cmd;
    enum mem_type_t mem;
    uint16_t start_address;
    uint16_t length;
};

struct remote_msg_boot_flash
{
    uint8_t address;
    uint8_t cmd;
    uint8_t data[REMOTE_MSGLEN-2];
};

#endif
