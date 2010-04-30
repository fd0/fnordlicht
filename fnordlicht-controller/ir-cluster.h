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

#include <stdint.h>

#ifndef __IR_CLUSTER_H
#define __IR_CLUSTER_H

/* search for (one-dimensional) clusters within data[],
 * consider only values at even positions */
uint8_t ir_cluster(uint16_t data[], uint8_t len, uint16_t cluster[], uint8_t max);

/* get cluster index belonging to data */
uint8_t ir_min_cluster(uint16_t data, uint16_t cluster[], uint8_t len);

#endif
