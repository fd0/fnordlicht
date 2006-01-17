/* fnordlicht rewritten in c
 * vim:fdm=marker ts=4 et ai
 */

/* define the cpu speed when using the fuse bits set by the
 * makefile (and documented in fuses.txt), using an external
 * crystal oscillator at 16MHz */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* debug defines */
#ifndef DEBUG
#define DEBUG 0
#endif

/* includes */
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

/* macros for extracting low and high byte */
#define LOW(x) (0x00ff & x)
#define HIGH(x) ((0xff00 & x) >> 8)

/* define uart baud rate (19200) and mode (8N1) */
#define UART_BAUDRATE 19200
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)
#define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)

/* possible pwm interrupts in a pwm cycle */
#define PWM_MAX_TIMESLOTS 5


/* structs {{{ */
struct Channel {
    uint8_t current;
    uint8_t target;
    union {
        uint16_t speed;
        struct {
            uint8_t speed_l;
            uint8_t speed_h;
        };
    };
    uint8_t flags;
    uint8_t remainder;
};

struct Timeslots {
    struct {
        uint8_t mask;
        uint16_t top;
    } slots[PWM_MAX_TIMESLOTS];

    uint8_t index;  /* current timeslot intex in the 'slots' array */
    uint8_t count;  /* number of entries in slots */
    uint8_t next_bitmask; /* next output bitmask, or signal for start or middle of pwm cycle */
};

/* }}} */


/* global variables {{{ */
volatile struct {
    uint8_t new_cycle:1;    /* set by pwm interrupt after burst, signals the beginning of a new pwm cycle to the main loop. */
    uint8_t last_pulse:1;   /* set by pwm interrupt after last interrupt in the current cycle, signals the main loop to rebuild the pwm timslot table */
    uint8_t pwm_overflow:1; /* set for pwm interrupt, when the next interrupt should dcrement TCNT1 */
} flags = {0, 0, 0};

struct Timeslots pwm;       /* pwm timeslots (the top values and masks for the timer1 interrupt) */
struct Channel channels[3]; /* current channel records */

/* }}} */


/** init the hardware uart */
static inline void init_uart(void) { /* {{{ */
    /* set baud rate */
    UBRRH = (uint8_t)(UART_UBRR >> 8);  /* high byte */
    UBRRL = (uint8_t)UART_UBRR;         /* low byte */

    /* set mode */
    UCSRC = UART_UCSRC;

    /* enable transmitter, receiver and receiver complete interrupt */
    UCSRB = _BV(TXEN) | _BV(RXEN) | _BV(RXCIE);

    /* send boot message */
    UDR = 'B';
}
/* }}} */

/** init output channels */
static inline void init_output(void) { /* {{{ */
    /* set all channels high -> leds off */
    PORTB = _BV(PB0) | _BV(PB1) | _BV(PB2);
    /* configure PB0-PB2 as outputs */
    DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);
}

/* }}} */

/** init timer 1 */
static inline void init_timer1(void) { /* {{{ */
    /* no prescaler, normal mode */
    TCCR1B = _BV(CS10);
    TCCR1A = 0;

    /* enable timer1 output compare 1a interrupt */
    TIMSK |= _BV(OCIE1A);
}

/* }}} */

/** timer1 output compare a interrupt */
SIGNAL(SIG_OUTPUT_COMPARE1A) { /* {{{ */
    /* check if we have an overflow interrupt */
    if (flags.pwm_overflow) {
        /* decrement counter register by 64k and 6,
         * because these instructions cost 6 cycles */
        TCNT1 -= 64000-6;

        /* reset overflow flag */
        flags.pwm_overflow = 0;

        if (pwm.next_bitmask == 0) {
            /* first interrupt */

            /* set portb to low, led on */
            PORTB &= ~_BV(0);

            UDR = 's';

            /* signal new cycle to main procedure */
            flags.new_cycle = 1;
        } else {
            UDR = 'm';
        }

    } else {
        /* normal interrupt here */

        PORTB |= _BV(0);

        UDR = 'n';
    }

    /* load new top and bitmask */

    OCR1A = pwm.slots[pwm.index].top;
    pwm.next_bitmask = pwm.slots[pwm.index].mask;

    /* if next top is 64000, we have an overflow timeslot */
    if (OCR1A == 64000)
        flags.pwm_overflow = 1;

    /* select next timeslot and trigger timeslot table rebuild if necessary */
    pwm.index++;

    if (pwm.index == pwm.count) {
        pwm.index = 0;
        flags.last_pulse = 1;
    }
}
/* }}} */

/** uart receive interrupt */
SIGNAL(SIG_UART_RECV) { /* {{{ */
    uint8_t data = UDR;

    switch (data) {
        case '+':
            pwm.slots[0].top += 1000;
            break;
        case '-':
            pwm.slots[0].top -= 1000;
            break;
    }
}
/* }}} */

/** main function
 */
int main(void) { /* {{{ */
    uint8_t i;

    init_output();
    init_uart();
    init_timer1();

    for (i=0; i<3; i++) {
        channels[i].current = 100;
        channels[i].target = 100;
        channels[i].speed = 0x0100;
        channels[i].flags = 0;
        channels[i].remainder = 0;
    }

    /* next interrupt is the first in a cycle, so set the bitmask to 0, so that the counter is reset */
    pwm.next_bitmask = 0;
    flags.pwm_overflow = 1;

    /* first timeslot, and 3 other timeslots are available */
    pwm.index = 0;
    pwm.count = 3;

    /* first interrupt after 2000 */
    pwm.slots[0].top = 2000;
    pwm.slots[0].mask = 0xff;

    /* next interrupt: middle */
    pwm.slots[1].top = 64000;
    pwm.slots[1].mask = 0xff;

    /* last interrupt: new start */
    pwm.slots[2].top = 64000;
    pwm.slots[2].mask = 0;

    /* load timer1 initial delay */
    OCR1A = 64000;

    /* enable interrupts globally */
    sei();

    while (1) {
        if (flags.new_cycle) {
            flags.new_cycle = 0;
        }

        if (flags.last_pulse) {
            flags.last_pulse = 0;

            pwm.next_bitmask = 0;
            pwm.index = 0;
            pwm.count = 3;

            flags.pwm_overflow = 1;
        }
    }
}
/* }}} */


