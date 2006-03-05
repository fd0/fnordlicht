/* vim:fdm=marker ts=4 et ai
 * {{{
 *         fnordlicht firmware next generation
 *
 *    for additional information please
 *    see http://koeln.ccc.de/prozesse/running/fnordlicht
 *
 * (c) by Alexander Neumann <alexander@bumpern.de>
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
 }}} */


#include "fifo.h"


inline uint8_t fifo_fill(volatile struct fifo_t *fifo)
/* {{{ */ {
    if (fifo->front >= fifo->back) {
        return fifo->front - fifo->back;
    } else {
        return (fifo->size) - (fifo->back - fifo->front + 1);
    }
} /* }}} */

inline uint8_t fifo_capacity(volatile struct fifo_t *fifo)
/* {{{ */ {
    return (fifo->size - fifo_fill(fifo) - 1);
} /* }}} */

inline void fifo_init(volatile struct fifo_t *fifo, uint8_t fifo_size)
/* {{{ */ {
    /* reset pointer */
    fifo->front = 0;
    fifo->back = 0;
    fifo->size = fifo_size;
} /* }}} */

inline void fifo_store(volatile struct fifo_t *fifo, uint8_t data)
/* {{{ */ {
    /* check if there is some space for saving a byte */
    if ( fifo_capacity(fifo) > 0) {
        /* safe byte */
        fifo->buffer[fifo->front] = data;

        /* calculate new front pointer */
        fifo->front = (fifo->front+1) & fifo->size;
    }
} /* }}} */

inline void fifo_store_buffer(volatile struct fifo_t *fifo, uint8_t data[])
/* {{{ */ {
    uint8_t i = 0;

    /* while there is still enough space in the fifo and we haven't reached the
     * end of the buffer, store byte */
    while (data[i] != 0 && fifo_capacity(fifo) > 0) {
        fifo_store(fifo, data[i++]);
    }
} /* }}} */

inline uint8_t fifo_load(volatile struct fifo_t *fifo)
/* {{{ */ {
    uint8_t data = 0;

    /* check if the fifo contains something */
    if ( fifo_fill(fifo) > 0) {

        /* load data */
        data = fifo->buffer[fifo->back];

        /* calculate new back pointer */
        fifo->back = (fifo->back+1) & fifo->size;
    }

    return data;
} /* }}} */

