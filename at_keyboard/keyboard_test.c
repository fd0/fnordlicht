
#define F_CPU 16000000L

#include "at_keyboard.h"

#define UART_BAUDRATE 19200

/* define uart baud rate (19200) and mode (8N1) */
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)
#define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)


/** init the hardware uart */
static inline void init_uart(void)
/*{{{*/ {
    /* set baud rate */
    UBRRH = (uint8_t)(UART_UBRR >> 8);  /* high byte */
    UBRRL = (uint8_t)UART_UBRR;         /* low byte */

    /* set mode */
    UCSRC = UART_UCSRC;

    /* enable transmitter, receiver and receiver complete interrupt */
    UCSRB = _BV(TXEN) | _BV(RXEN) | _BV(RXCIE);

    /* send boot message */
    UDR = 'B';

    /* wait until boot message has been sent over the wire */
    while (!(UCSRA & (1<<UDRE)));
} /* }}} */


/** interrupts*/

/** uart receive interrupt */
SIGNAL(SIG_UART_RECV)
/*{{{*/ {
    uint8_t data = UDR;

    switch (data) {
        case 's': send_byte_to_keyboard(0xFF);
                  break;
    }
} /*}}}*/


int main(void) {
    init_uart();
    init_at_keyboard();

    sei();

    DDRB = 0xff;
    PORTB = 0xff;

    while (1) {
        if (global_keyboard.receive_complete) {
            global_keyboard.receive_complete = 0;

            UDR = global_keyboard.received_scancode;
            while (!(UCSRA & (1<<UDRE)));
        }
    }
}
