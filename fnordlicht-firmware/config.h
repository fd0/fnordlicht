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

/*
 * ATTENTION:
 * This file is under version control.  Please use the file "config.mk" (which
 * is not under version control) for configuring the fnordlicht firmware.
 * A file with all the default values will be created by running 'make' the
 * first time.
 */

#ifndef _FNORDLICHT_CONFIG_H
#define _FNORDLICHT_CONFIG_H

/* debug defines */
#ifndef CONFIG_DEBUG
#define CONFIG_DEBUG 0
#endif

/* include the script interpreter per default */
#ifndef CONFIG_SCRIPT
#define CONFIG_SCRIPT 1
#endif

/* define default startup script (0 is colorwheel, 1 is random) */
#ifndef CONFIG_SCRIPT_DEFAULT
#define CONFIG_SCRIPT_DEFAULT 0
#endif

/* include uart support per default */
#ifndef CONFIG_SERIAL
#define CONFIG_SERIAL 1
#endif

/* include remote command support per default */
#ifndef CONFIG_REMOTE
#define CONFIG_REMOTE 1
#endif

/* secondary output pins */
#ifndef CONFIG_SECONDARY_PWM
#define CONFIG_SECONDARY_PWM 1
#endif

/* set default baudrate */
#ifndef CONFIG_SERIAL_BAUDRATE
#define CONFIG_SERIAL_BAUDRATE 19200
#endif

/* disable static master mode by default (still check jumper) */
#ifndef CONFIG_STATIC_MASTER
#define CONFIG_STATIC_MASTER 0
#endif

/* check if hardware is valid */
#if defined(HARDWARE_fnordlicht)
    /* specific settings for old fnordlicht hardware */
    #if !defined(PWM_INVERTED)
    #define PWM_INVERTED
    #endif

    /* disable secondary pwm for old fnordlicht hardware */
    #if defined(CONFIG_SECONDARY_PWM) && CONFIG_SECONDARY_PWM
    #undef CONFIG_SECONDARY_PWM
    #define CONFIG_SECONDARY_PWM 0
    #endif

#elif defined(HARDWARE_fnordlichtmini)
    /* specific settings for fnordlichtmini hardware */

#else
#error "unknown HARDWARE platform!"
#endif

#endif /* _FNORDLICHT_CONFIG_H */
