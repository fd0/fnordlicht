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

#ifndef COLOR_H
#define COLOR_H

#if PWM_CHANNELS != 3
#error "PWM_CHANNELS is not 3, this is unsupported!"
#endif

struct rgb_color_t
{
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t rgb[3];
    };
};

struct hsv_color_t
{
    union {
        struct {
            uint16_t hue;
            uint8_t value;
            uint8_t saturation;
        };
        uint8_t hsv[4];
    };
};

struct dual_color_t
{
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t rgb[3];
    };
    union {
        struct {
            uint16_t hue;
            uint8_t value;
            uint8_t saturation;
        };
        uint8_t hsv[4];
    };
};

struct union_color_t
{
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t rgb[3];
        struct {
            uint16_t hue;
            uint8_t value;
            uint8_t saturation;
        };
        uint8_t hsv[4];
    };
};

#endif
