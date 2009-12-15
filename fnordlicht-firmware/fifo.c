/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
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

#include "fifo.h"

void fifo_init(fifo_t *f)
{
    f->read = 0;
    f->write = 0;
}

void fifo_enqueue(fifo_t *f, fifo_content_t data)
{
    f->buffer[f->write] = data;
    f->write = (f->write + 1) % CONFIG_FIFO_SIZE;
}

fifo_content_t fifo_dequeue(fifo_t *f)
{
    fifo_content_t data = f->buffer[f->read];
    f->read = (f->read + 1) % CONFIG_FIFO_SIZE;
    return data;
}

fifo_size_t fifo_fill(fifo_t *f)
{
    if (f->write >= f->read)
        return f->write - f->read;
    else
        return CONFIG_FIFO_SIZE - (f->read - f->write);
}

bool fifo_empty(fifo_t *f)
{
    return f->read == f->write;
}

bool fifo_full(fifo_t *f)
{
    return fifo_fill(f) == CONFIG_FIFO_SIZE-1;
}
