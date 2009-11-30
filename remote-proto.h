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

#ifndef __REMOTE_PROTO
#define __REMOTE_PROTO 1

#include "color.h"

#define REMOTE_MSG_LEN 15
#define REMOTE_SYNC_LEN 15

/* normal commands */
#define REMOTE_CMD_FADE_RGB         0x02
#define REMOTE_CMD_FADE_HSV         0x03
#define REMOTE_CMD_SAVE_RGB         0x04
#define REMOTE_CMD_SAVE_HSV         0x05
#define REMOTE_CMD_SAVE_CURRENT     0x06
#define REMOTE_CMD_CONFIG_OFFSETS   0x07
#define REMOTE_CMD_START_PROGRAM    0x09
#define REMOTE_CMD_STOP             0x0A
#define REMOTE_CMD_MODIFY_CURRENT   0x0B
#define REMOTE_CMD_PULL_INT         0x0C
#define REMOTE_CMD_CONFIG_STARTUP   0x0D

#define REMOTE_CMD_RESYNC           0x1b

/* max mode for startup configuration is 2 */
#define REMOTE_STARTUP_MAX_MODE     2
/* maximum parameter size (for copy loop), size of structure storage_config_t,
 * minus 1 for startup_mode enum */
#define REMOTE_STARTUP_MAX_PARAMSIZE 11

/* bootloader commands */
#define REMOTE_CMD_BOOTLOADER       0x80

#define REMOTE_ADDR_BROADCAST 0xff

/* normal commands */
struct remote_msg_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t data[REMOTE_MSG_LEN-2];
};

struct remote_msg_fade_rgb_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    struct rgb_color_t color;
};

struct remote_msg_fade_hsv_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    struct hsv_color_t color;
};

struct remote_msg_save_rgb_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t slot;
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
    struct rgb_color_t color;
};

struct remote_msg_save_hsv_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t slot;
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
    struct hsv_color_t color;
};

struct remote_msg_save_current_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t slot;
    uint8_t step;
    uint8_t delay;
    uint16_t pause;
};

struct remote_msg_config_offsets_t
{
    uint8_t address;
    uint8_t cmd;
    int8_t step;
    int8_t delay;
    int16_t hue;
    uint8_t saturation;
    uint8_t value;
};

struct remote_msg_start_program_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t script;
    uint8_t parameters[PROGRAM_PARAMETER_SIZE];
};

struct remote_msg_stop_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t fade;
};

struct remote_msg_modify_current_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t step;
    uint8_t delay;
    struct rgb_color_offset_t rgb;
    struct hsv_color_offset_t hsv;
};

struct remote_msg_pull_int_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t delay;
};

enum startup_mode_t
{
    STARTUP_PROGRAM = 0,
    STARTUP_STATIC = 1,
};

union startup_parameters_t
{
    /* raw access to data, size: 11 byte */
    uint8_t raw[11];

    /* structure for startup_mode == STARTUP_PROGRAM
     * size: 11 byte */
    struct {
        uint8_t program;
        uint8_t program_parameters[PROGRAM_PARAMETER_SIZE];
    };

    /* structure for startup_mode == STARTUP_STATIC
     * size: 5 byte */
    struct {
        uint8_t step;
        uint8_t delay;
        struct rgb_color_t color;
    };
};

struct remote_msg_config_startup_t
{
    uint8_t address;
    uint8_t cmd;
    enum startup_mode_t mode;
    union startup_parameters_t params;
};

/* bootloader commands */
enum mem_type_t
{
    MEM_FLASH = 0,
    MEM_EEPROM = 1,
};

struct remote_msg_boot_config_t
{
    uint8_t address;
    uint8_t cmd;
    enum mem_type_t mem;
    uint16_t start_address;
    uint16_t length;
};

struct remote_msg_boot_flash_t
{
    uint8_t address;
    uint8_t cmd;
    uint8_t data[REMOTE_MSG_LEN-2];
};

#endif
