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
#include <avr/pgmspace.h>

/* macros for extracting low and high byte */
#define LOW(x) (uint8_t)(0x00ff & x)
#define HIGH(x) (uint8_t)((0xff00 & x) >> 8)

/* define uart baud rate (19200) and mode (8N1) */
#define UART_BAUDRATE 19200
#define UART_UCSRC _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1)
#define UART_UBRR (F_CPU/(UART_BAUDRATE * 16L)-1)

/* possible pwm interrupts in a pwm cycle */
#define PWM_MAX_TIMESLOTS 5

#define PWM_CHANNELS 3
#define PWM_BURST_LEVELS 16


/* structs {{{ */
struct Channel {
    uint8_t brightness;
    uint8_t target_brightness;
    union {
        uint16_t speed;
        struct {
            uint8_t speed_l;
            uint8_t speed_h;
        };
    };
    uint8_t flags;
    uint8_t remainder;
    uint8_t mask;
};

struct Timeslots {
    struct {
        uint8_t mask;
        uint16_t top;
    } slots[PWM_MAX_TIMESLOTS];

    uint8_t index;  /* current timeslot intex in the 'slots' array */
    uint8_t count;  /* number of entries in slots */
    uint8_t next_bitmask; /* next output bitmask, or signal for start or middle of pwm cycle */
    uint8_t initial_bitmask; /* output mask set at beginning */
};

/* }}} */

/* timer top values for 256 brightness levels (stored in flash) {{{ */
const uint16_t timeslot_table[] PROGMEM = {               \
      2,     8,    18,    31,    49,    71,    96,   126, \
    159,   197,   238,   283,   333,   386,   443,   504, \
    569,   638,   711,   787,   868,   953,  1041,  1134, \
   1230,  1331,  1435,  1543,  1655,  1772,  1892,  2016, \
   2144,  2276,  2411,  2551,  2695,  2842,  2994,  3150, \
   3309,  3472,  3640,  3811,  3986,  4165,  4348,  4535, \
   4726,  4921,  5120,  5323,  5529,  5740,  5955,  6173, \
   6396,  6622,  6852,  7087,  7325,  7567,  7813,  8063, \
   8317,  8575,  8836,  9102,  9372,  9646,  9923, 10205, \
  10490, 10779, 11073, 11370, 11671, 11976, 12285, 12598, \
  12915, 13236, 13561, 13890, 14222, 14559, 14899, 15244, \
  15592, 15945, 16301, 16661, 17025, 17393, 17765, 18141, \
  18521, 18905, 19293, 19685, 20080, 20480, 20884, 21291, \
  21702, 22118, 22537, 22960, 23387, 23819, 24254, 24693, \
  25135, 25582, 26033, 26488, 26946, 27409, 27876, 28346, \
  28820, 29299, 29781, 30267, 30757, 31251, 31750, 32251, \
  32757, 33267, 33781, 34299, 34820, 35346, 35875, 36409, \
  36946, 37488, 38033, 38582, 39135, 39692, 40253, 40818, \
  41387, 41960, 42537, 43117, 43702, 44291, 44883, 45480, \
  46080, 46684, 47293, 47905, 48521, 49141, 49765, 50393, \
  51025, 51661, 52300, 52944, 53592, 54243, 54899, 55558, \
  56222, 56889, 57560, 58235, 58914, 59598, 60285, 60975, \
  61670, 62369, 63072, 63779,   489,  1204,  1922,  2645, \
   3371,  4101,  4836,  5574,  6316,  7062,  7812,  8566, \
   9324, 10085, 10851, 11621, 12394, 13172, 13954, 14739, \
  15528, 16322, 17119, 17920, 18725, 19534, 20347, 21164, \
  21985, 22810, 23638, 24471, 25308, 26148, 26993, 27841, \
  28693, 29550, 30410, 31274, 32142, 33014, 33890, 34770, \
  35654, 36542, 37433, 38329, 39229, 40132, 41040, 41951, \
  42866, 43786, 44709, 45636, 46567, 47502, 48441, 49384, \
  50331, 51282, 52236, 53195, 54158, 55124, 56095, 57069, \
  58047, 59030, 60016, 61006, 62000, 62998 };

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

/* header */
static inline void init_uart(void);
static inline void init_output(void);
static inline void init_timer1(void);
void update_pwm_timeslots(void);
static inline void init_pwm(void);
inline void do_fading(void);


/** init the hardware uart */
void init_uart(void) { /* {{{ */
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
}
/* }}} */

/** init output channels */
void init_output(void) { /* {{{ */
    /* set all channels high -> leds off */
    PORTB = _BV(PB0) | _BV(PB1) | _BV(PB2);
    /* configure PB0-PB2 as outputs */
    DDRB = _BV(PB0) | _BV(PB1) | _BV(PB2);
}

/* }}} */

/** init timer 1 */
void init_timer1(void) { /* {{{ */
    /* no prescaler, normal mode */
    TCCR1B = _BV(CS10);
    TCCR1A = 0;

    /* enable timer1 output compare 1a interrupt */
    TIMSK |= _BV(OCIE1A);
}

/* }}} */

/** init pwm */
void init_pwm(void) { /* {{{ */
    uint8_t i;

    for (i=0; i<3; i++) {
        channels[i].brightness = 0;
        channels[i].target_brightness = 0;
        channels[i].speed = 0x0100;
        channels[i].flags = 0;
        channels[i].remainder = 0;
        channels[i].mask = _BV(i);
    }

    //channels[0].target_brightness = 192;
    //channels[1].target_brightness = 20;
    //channels[2].target_brightness = 182;

    channels[0].brightness = 30;
    channels[1].brightness = 0;
    channels[2].brightness = 0;
    channels[0].target_brightness=channels[0].brightness;
    channels[1].target_brightness=channels[1].brightness;
    channels[2].target_brightness=channels[2].brightness;

    update_pwm_timeslots();
}

/* }}} */

/** update pwm timeslot table */
void update_pwm_timeslots(void) { /* {{{ */
    uint8_t sorted[PWM_CHANNELS] = { 0, 1, 2 };
    uint8_t i, j;
    uint8_t mask = 0;
    uint8_t last_brightness = 0;

    /* sort channels according to the current brightness */
    for (i=0; i<PWM_CHANNELS; i++) {
        for (j=i+1; j<PWM_CHANNELS; j++) {
            if (channels[sorted[j]].brightness < channels[sorted[i]].brightness) {
                uint8_t temp;

                temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    /* timeslot index */
    j = 0;

    /* calculate timeslots and masks */
    for (i=0; i < PWM_CHANNELS; i++) {

        /* check if a timeslot is needed */
        if (channels[sorted[i]].brightness > 0 && channels[sorted[i]].brightness < 255) {
            /* if the next timeslot will be after the middle of the pwm cycle, insert the middle interrupt */
            if (last_brightness < 181 && channels[sorted[i]].brightness >= 181) {
                /* middle interrupt: top 64k and mask 0xff */
                pwm.slots[j].top = 64000;
                pwm.slots[j].mask = 0xff;
                j++;
            }

            /* insert new timeslot if brightness is new */
            if (channels[sorted[i]].brightness > last_brightness) {

                /* remember mask and brightness for next timeslot */
                mask |= channels[sorted[i]].mask;
                last_brightness = channels[sorted[i]].brightness;

                /* allocate new timeslot */
                pwm.slots[j].top = pgm_read_word(&timeslot_table[channels[sorted[i]].brightness - 1 ]);
                pwm.slots[j].mask = mask;
                j++;
            } else {
            /* change mask of last-inserted timeslot */
                mask |= channels[sorted[i]].mask;
                pwm.slots[j-1].mask = mask;
            }
        }
    }

    /* if all interrupts happen before the middle interrupt, insert it here */
    if (last_brightness < 181) {
        /* middle interrupt: top 64k and mask 0xff */
        pwm.slots[j].top = 64000;
        pwm.slots[j].mask = 0xff;
        j++;
    }

    /* insert last interrupt: new start */
    pwm.slots[j].top = 64000;
    pwm.slots[j].mask = 0;
    j++;

    /* reset pwm structure */
    pwm.index = 0;
    pwm.count = j;

    /* next interrupt is the first in a cycle, so set the bitmask to 0, so that the counter is reset */
    pwm.next_bitmask = 0;

    /* calculate initial bitmask */
    pwm.initial_bitmask = 0xff;
    for (i=0; i < PWM_CHANNELS; i++)
        if (channels[i].brightness > 0)
            pwm.initial_bitmask &= ~channels[i].mask;

    /* next interrupt is an overflow interrupt */
    flags.pwm_overflow = 1;
}

/* }}} */

/** fade any channels not already at their target brightness */
void do_fading(void) { /* {{{ */
    uint8_t i;

    for (i=0; i<PWM_CHANNELS; i++) {
        if (channels[i].brightness < channels[i].target_brightness) {
            channels[i].brightness++;
        } else if (channels[i].brightness > channels[i].target_brightness) {
            channels[i].brightness--;
        }
    }
}

/* }}} */


/** timer1 output compare a interrupt */
SIGNAL(SIG_OUTPUT_COMPARE1A) { /* {{{ */
    /* check if we have an overflow interrupt */
    if (flags.pwm_overflow) {
        /* decrement counter register by 64k and 6,
         * because these instructions cost 6 cycles */
        TCNT1 -= 64000-6;

        if (pwm.next_bitmask == 0) {
            /* first interrupt */

            /* output initial values */
            PORTB = pwm.initial_bitmask;
        } else {
            /* middle, nothing to do here so far */
        }

        /* reset overflow flag */
        flags.pwm_overflow = 0;

        /* signal new cycle to main procedure */
        flags.new_cycle = 1;
    } else {
        /* normal interrupt here */

        PORTB |= pwm.next_bitmask;
    }

    /**
     * prepare next timeslot
     */

    /* if next timeslot would happen too fast or has already happened, just
     * spinlock and call interrupt handler again */
    while (pwm.slots[pwm.index].top - TCNT1 < 100 || pwm.slots[pwm.index].top <
            TCNT1)
    {
        /* spin until timer interrupt is near enough */
        while (pwm.slots[pwm.index].top > TCNT1) {
            //asm volatile ("nop");
        }

        /* output value */
        PORTB |= pwm.slots[pwm.index].mask;

        /* we can safely just increment the index, as the will always be at
         * least one interrupt after this (middle) */
        pwm.index++;

    }

    /* if we have enough time, just set new top and let the timer do all the work */
    {
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
}
/* }}} */

/** uart receive interrupt */
SIGNAL(SIG_UART_RECV) { /* {{{ */
    uint8_t data = UDR;

    switch (data) {
        case '1':
            channels[0].target_brightness-=1;
            break;
        case '4':
            channels[0].target_brightness+=1;
            break;
        case '2':
            channels[1].target_brightness-=1;
            break;
        case '5':
            channels[1].target_brightness+=1;
            break;
        case '3':
            channels[2].target_brightness-=1;
            break;
        case '6':
            channels[2].target_brightness+=1;
            break;
        case '0':
            channels[0].target_brightness=0;
            channels[1].target_brightness=0;
            channels[2].target_brightness=0;
            break;
        case '=':
            channels[0].target_brightness=channels[0].brightness;
            channels[1].target_brightness=channels[1].brightness;
            channels[2].target_brightness=channels[2].brightness;
            break;
    }
}
/* }}} */

/** main function
 */
int main(void) { /* {{{ */
    init_output();
    init_uart();
    init_timer1();
    init_pwm();

    /* load timer1 initial delay */
    OCR1A = 64000;

    /* enable interrupts globally */
    sei();

    while (1) {
        if (flags.new_cycle) {
            flags.new_cycle = 0;

            do_fading();
        }

        if (flags.last_pulse) {
            flags.last_pulse = 0;

            update_pwm_timeslots();
        }
    }
}
/* }}} */


