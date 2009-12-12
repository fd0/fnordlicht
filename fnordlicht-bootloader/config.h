/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlichtmini serial bootloader
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* configure bootloader buffer size */
#ifndef CONFIG_BOOTLOADER_BUFSIZE
#define CONFIG_BOOTLOADER_BUFSIZE 512
#endif

/* configure exit delay (in 50ms) */
#ifndef CONFIG_EXIT_DELAY
#define CONFIG_EXIT_DELAY 2
#endif

#endif
