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

#ifndef PWM_H
#define PWM_H

#include "timer.h"

/* possible pwm interrupts in a pwm cycle */
#define PWM_MAX_TIMESLOTS (PWM_CHANNELS+1)

#if PWM_CHANNELS != 3
#error "PWM_CHANNELS is not 3, this is unsupported!"
#endif

struct rgb_color_t {
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t rgb[3];
    };
};

struct hsv_color_t {
    union {
        struct {
            uint8_t hue;
            uint8_t value;
            uint8_t saturation;
        };
        uint8_t hsv[3];
    };
};

struct dual_color_t {
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
            uint8_t hue;
            uint8_t value;
            uint8_t saturation;
        };
        uint8_t hsv[3];
    };
};

struct union_color_t {
    union {
        struct {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        };
        uint8_t rgb[3];
        struct {
            uint8_t hue;
            uint8_t value;
            uint8_t saturation;
        };
        uint8_t hsv[3];
    };
};

struct global_pwm_t
{
    /* current color */
    struct dual_color_t current;

    /* target for fading engine */
    struct union_color_t target;

    /* set to activate hsv color fading engine */
    bool use_hsv;

    /* delay and step for fading engine */
    uint8_t fade_delay[PWM_CHANNELS];
    uint8_t fade_step[PWM_CHANNELS];
};

extern volatile struct global_pwm_t global_pwm;

/* prototypes */
void pwm_init(void);
void pwm_poll(void);
void pwm_poll_fading(void);

void pwm_fade_rgb(struct rgb_color_t *color, uint8_t step, uint8_t delay);
void pwm_fade_hsv(struct hsv_color_t *color, uint8_t step, uint8_t delay);

/* return true if fading process is complete */
bool pwm_target_reached(void);

/* convert hsv to rgb color */
void pwm_hsv2rgb(struct dual_color_t *color);

/* copy current hsv to rgb color */
void pwm_copyhsv2rgb(void);

#endif
