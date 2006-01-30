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


inline uint8_t fifo_fill(struct fifo_t *fifo)
/* {{{ */ {
    if (fifo->front <= fifo->back) {
        return fifo->back - fifo->front;
    } else {
        return fifo->front - fifo->back;
    }
} /* }}} */

inline void fifo_init(struct fifo_t *fifo)
/* {{{ */ {
    /* reset pointer */
    fifo->front = 0;
    fifo->back = 0;
} /* }}} */

inline void fifo_store(struct fifo_t *fifo, uint8_t data)
/* {{{ */ {
    /* check if there is some space for saving a byte */
    if ( fifo_fill(fifo) < (UART_FIFO_SIZE-1)) {

        /* safe byte */
        fifo->buffer[fifo->front] = data;

        /* calculate new front pointer */
        fifo->front = (fifo->front+1) & ~(UART_FIFO_SIZE-1);
    }
} /* }}} */

inline uint8_t fifo_load(struct fifo_t *fifo)
/* {{{ */ {
    uint8_t data = 0;

    /* check if the fifo contains something */
    if ( fifo_fill(fifo) > 0) {

        /* load data */
        data = fifo->buffer[fifo->back];

        /* calculate new back pointer */
        fifo->back = (fifo->back+1) & ~(UART_FIFO_SIZE-1);
    }

    return data;
} /* }}} */

