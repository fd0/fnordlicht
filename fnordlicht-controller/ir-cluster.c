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

#include "ir-cluster.h"

/* return the position of the (first) minimum value within data[],
 * considering only values at even positions */
static uint8_t minimum(uint16_t data[], uint8_t len)
{
    uint8_t min = 0;

    for (uint8_t i = 0; i < 2*len; i += 2) {
        if (data[i] < data[min])
            min = i;
    }

    return min;
}

/* search next bigger value (starting at data[from]) within data[], just
 * considering the even values, return -1 on error (no bigger value found) */
static int8_t next(uint16_t data[], uint8_t len, uint8_t from)
{
    len *= 2;

    uint16_t old = data[from];

    /* test if the same value appears again within data[] after from */
    for (uint8_t i = from+2; i < len; i += 2) {
        if (data[i] == old)
            /* found the same value again, at pos i */
            return i;
    }

    /* else search for the next bigger value */
    int16_t pos = -1;
    for (uint8_t i = 0; i < len; i += 2) {

        /* if the current value is lower than the old value, try next */
        if (data[i] <= old)
            continue;

        /* if we haven't found a bigger value yet, or if the current value
         * is smaller than the value we looked at before,
         * consider the current position as the next value */
        if (pos < 0 || data[i] < data[pos])
            pos = i;
    }

    return pos;
}

/* search for (one-dimensional) clusters within data[],
 * consider only values at even positions */
uint8_t ir_cluster(uint16_t data[], uint8_t len, uint16_t cluster[], uint8_t max)
{
    uint8_t cindex = 0;

    /* search minimum within data[] */
    uint8_t pos = minimum(data, len);

    /* initialize mean value */
    uint32_t mean = data[pos];
    uint8_t count = 1;

    /* iterate over data[], processing the values (at even positions) in
     * ascending order */
    for (uint8_t i = 0; i < len-1; i++) {

        /* search position of the next element within data[] */
        uint8_t nextpos = next(data, len, pos);

        /* as a shortcut, name values a and b */
        uint16_t a = data[pos];
        uint16_t b = data[nextpos];

        /* check if b > 1.5*a */
        a += a/2;
        if (b > a) {

            /* reached a step, found a cluster */
            mean /= count;
            cluster[cindex++] = (uint16_t)mean;

            /* stop processing since max cluster values is reached */
            if (cindex == max)
                return max;

            /* reset mean value */
            mean = 0;
            count = 0;
        }

        /* add value to mean */
        mean += b;
        count++;

        /* advance position */
        pos = nextpos;
    }

    /* if there are some values left in mean, this is the last cluster */
    if (count > 0) {
        mean /= count;
        cluster[cindex++] = (uint16_t)mean;
    }

    return cindex;
}

/* get cluster index belonging to data */
uint8_t ir_min_cluster(uint16_t data, uint16_t cluster[], uint8_t len)
{
    uint8_t min = 0;
    uint16_t diff = 0xffff;

    /* iterate over possible clusters */
    for (uint8_t i = 0; i < len; i++) {
        uint16_t curdiff;

        /* get positive difference */
        if (data < cluster[i])
            curdiff = cluster[i] - data;
        else
            curdiff = data - cluster[i];

        /* if difference is lower, remember this cluster */
        if (curdiff < diff) {
            diff = curdiff;
            min = i;
        } else
            /* stop here, since difference is larger than for the last cluster
             * (cluster[] is ordered ascending */
            break;
    }

    return min;
}
