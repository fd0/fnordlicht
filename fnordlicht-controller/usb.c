/* vim:ts=4 sts=4 et tw=80
 *
 *         fnordlicht firmware
 *
 *    for additional information please
 *    see http://lochraster.org/fnordlichtmini
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

/* includes */
#include "globals.h"
#include "../common/io.h"

#include <stdint.h>
#include <stdbool.h>

#include "../common/common.h"
#include "usbdrv/usbdrv.h"
#include "timer.h"
#include "uart.h"

static bool usb_status;

 /* supply custom usbDeviceConnect() and usbDeviceDisconnect() macros
 * which turn the interrupt on and off at the right times,
 * and prevent the execution of an interrupt while the pullup resistor
 * is switched off */
#ifdef USB_CFG_PULLUP_IOPORTNAME
#undef usbDeviceConnect
#define usbDeviceConnect()      do { \
                                    USB_PULLUP_DDR |= (1<<USB_CFG_PULLUP_BIT); \
                                    USB_PULLUP_OUT |= (1<<USB_CFG_PULLUP_BIT); \
                                    USB_INTR_ENABLE |= (1 << USB_INTR_ENABLE_BIT); \
                                   } while(0);
#undef usbDeviceDisconnect
#define usbDeviceDisconnect()   do { \
                                    USB_INTR_ENABLE &= ~(1 << USB_INTR_ENABLE_BIT); \
                                    USB_PULLUP_DDR &= ~(1<<USB_CFG_PULLUP_BIT); \
                                    USB_PULLUP_OUT &= ~(1<<USB_CFG_PULLUP_BIT); \
                                   } while(0);
#endif

#define USBRQ_SEND 1

struct usb_state_t {
    enum {
        USB_STATE_IDLE = 0,
        USB_STATE_SEND = 1,
    } state;

    uint8_t bytes_remaining;
};

struct usb_state_t usb;

USB_PUBLIC usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *req = (void *)data;
    usbMsgLen_t len = 0;
    static uint8_t buf[8];

    /* set global data pointer to local buffer */
    usbMsgPtr = buf;

    if (req->bRequest == USBRQ_SEND && req->wLength.word > 0) {
        usb.state = USB_STATE_SEND;
        if (req->wLength.word < 256) {
            usb.bytes_remaining = req->wLength.bytes[0];
            return USB_NO_MSG;
        } else {
            /* too long */
            buf[0] = 1;
            len = 1;
        }
    }

    return len;
}

uchar usbFunctionWrite(uchar *data, uchar len)
{
    if (len > usb.bytes_remaining)
        len = usb.bytes_remaining;

    usb.bytes_remaining -= len;

    while (len--) {
        /* ugly hack: busy wait until the fifo is ready for another byte... */
        while (fifo_full(&global_uart.tx));
        uart_putc(*data++);
    }

    if (usb.bytes_remaining == 0) {
        return true;
        usb.state == USB_STATE_IDLE;
    } else
        return len;
}

void usb_init(void)
{
    usbInit();
}

void usb_enable(void)
{
    usbDeviceConnect();
    usb_status = true;
}

void usb_disable(void)
{
    usbDeviceDisconnect();
    usb_status = false;
}

bool usb_enabled(void)
{
    return usb_status;
}

void usb_poll(void)
{
    if (usb_status)
        usbPoll();
}
