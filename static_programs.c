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

#include "static_programs.h"
#include "color.h"
#include "pwm.h"

#if CONFIG_SCRIPT

PT_THREAD(program_colorwheel(struct process_t *process))
{
    PT_BEGIN(&process->pt);

    static struct hsv_color_t c;

    c.hue = 0;
    c.value = 255;
    c.saturation = 255;

    while (1) {
        /* set new color */
        pwm_fade_hsv(&c, 1, 2);
        c.hue += 45;

        /* wait until target reached */
        PT_WAIT_UNTIL(&process->pt, pwm_target_reached());
    }

    PT_END(&process->pt);
}

#endif
