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

#ifndef __REMOTE_H
#define __REMOTE_H

#include <stdint.h>

struct remote_offsets_t
{
    int8_t step;
    int8_t delay;
    int16_t hue;
    uint8_t saturation;
    uint8_t value;
};

struct global_remote_t {
    struct remote_offsets_t offsets;
};

extern struct global_remote_t global_remote;

/* we depend on serial uart */
#if !CONFIG_SERIAL || !CONFIG_REMOTE

#define remote_init(...)
#define remote_poll(...)
#define remote_address(...) 0

#else

void remote_init(void);
void remote_poll(void);
uint8_t remote_address(void);

#endif
#endif
