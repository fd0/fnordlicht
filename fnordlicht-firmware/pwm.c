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

/* includes */
#include "globals.h"
#include "../common/io.h"

#include <stdint.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "../common/common.h"
#include "pwm.h"
#include "timer.h"
#include "remote.h"

/* abbreviations for port, ddr and pin */
#define P_PORT _OUTPORT(PWM_PORT)
#define P_DDR _DDRPORT(PWM_PORT)
#define P2_PORT _OUTPORT(PWM2_PORT)
#define P2_DDR _DDRPORT(PWM2_PORT)

/* TYPES AND PROTOTYPES */
#define PWM_MAX_TIMESLOTS (2*(PWM_CHANNELS+2))

/* encapsulates all pwm data including timeslot and output mask array */
struct timeslot_t
{
    uint8_t mask;
    uint16_t top;
};

struct timeslots_t
{
    /* store timslots in a queue */
    struct timeslot_t slot[PWM_MAX_TIMESLOTS];

    uint8_t read;   /* current read head in 'slot' array */
    uint8_t write;  /* current write head in 'slot' array */
};

/* internal data for the fading engine */
struct fading_engine_t
{
    /* a timer for each channel */
    timer_t timer[PWM_CHANNELS];

    /* and a bitmask, which stands for 'timer is running' */
    uint8_t running;
};

/* timer top values for 256 brightness levels (stored in flash) */
static const uint16_t timeslot_table[] PROGMEM =
{
      2,     8,    18,    31,    49,    71,    96,   126,
    159,   197,   238,   283,   333,   386,   443,   504,
    569,   638,   711,   787,   868,   953,  1041,  1134,
   1230,  1331,  1435,  1543,  1655,  1772,  1892,  2016,
   2144,  2276,  2411,  2551,  2695,  2842,  2994,  3150,
   3309,  3472,  3640,  3811,  3986,  4165,  4348,  4535,
   4726,  4921,  5120,  5323,  5529,  5740,  5955,  6173,
   6396,  6622,  6852,  7087,  7325,  7567,  7813,  8063,
   8317,  8575,  8836,  9102,  9372,  9646,  9923, 10205,
  10490, 10779, 11073, 11370, 11671, 11976, 12285, 12598,
  12915, 13236, 13561, 13890, 14222, 14559, 14899, 15244,
  15592, 15945, 16301, 16661, 17025, 17393, 17765, 18141,
  18521, 18905, 19293, 19685, 20080, 20480, 20884, 21291,
  21702, 22118, 22537, 22960, 23387, 23819, 24254, 24693,
  25135, 25582, 26033, 26488, 26946, 27409, 27876, 28346,
  28820, 29299, 29781, 30267, 30757, 31251, 31750, 32251,
  32757, 33267, 33781, 34299, 34820, 35346, 35875, 36409,
  36946, 37488, 38033, 38582, 39135, 39692, 40253, 40818,
  41387, 41960, 42537, 43117, 43702, 44291, 44883, 45480,
  46080, 46684, 47293, 47905, 48521, 49141, 49765, 50393,
  51025, 51661, 52300, 52944, 53592, 54243, 54899, 55558,
  56222, 56889, 57560, 58235, 58914, 59598, 60285, 60975,
  61670, 62369, 63072, 63779,   489,  1204,  1922,  2645,
   3371,  4101,  4836,  5574,  6316,  7062,  7812,  8566,
   9324, 10085, 10851, 11621, 12394, 13172, 13954, 14739,
  15528, 16322, 17119, 17920, 18725, 19534, 20347, 21164,
  21985, 22810, 23638, 24471, 25308, 26148, 26993, 27841,
  28693, 29550, 30410, 31274, 32142, 33014, 33890, 34770,
  35654, 36542, 37433, 38329, 39229, 40132, 41040, 41951,
  42866, 43786, 44709, 45636, 46567, 47502, 48441, 49384,
  50331, 51282, 52236, 53195, 54158, 55124, 56095, 57069,
  58047, 59030, 60016, 61006, 62000, 62998 };

/* GLOBAL VARIABLES */
struct global_pwm_t global_pwm;
static struct timeslots_t timeslots;
static struct fading_engine_t fading;

/* next output bitmask */
static volatile uint8_t pwm_next_bitmask;

/* FUNCTIONS AND INTERRUPTS */
/* prototypes */
void update_pwm_timeslots(struct rgb_color_t *target);
void update_rgb(uint8_t c);
void enqueue_timeslot(uint8_t mask, uint16_t top);
void dequeue_timeslot(struct timeslot_t *d);
void update_last_timeslot(uint8_t mask);
uint8_t timeslots_fill(void);

/* initialize pwm hardware and structures */
void pwm_init(void)
{
    /* init output pins */

#ifdef PWM_INVERTED
    /* set all pins high -> leds off */
    P_PORT |= PWM_CHANNEL_MASK;
#else
    /* set all pins low -> leds off */
    P_PORT &= ~(PWM_CHANNEL_MASK);
#endif

#if CONFIG_SECONDARY_PWM
#ifdef PWM_INVERTED
    /* set all pins high -> leds off */
    P2_PORT |= PWM2_CHANNEL_MASK;
#else
    /* set all pins low -> leds off */
    P2_PORT &= ~(PWM2_CHANNEL_MASK);
#endif
#endif

    /* configure pins as outputs */
    P_DDR |= PWM_CHANNEL_MASK;

#if CONFIG_SECONDARY_PWM
    P2_DDR |= PWM2_CHANNEL_MASK;
#endif

    /* initialize timer 1 */

    /* no prescaler, CTC mode */
    TCCR1B = _BV(CS10) | _BV(WGM12);

    /* enable timer1 overflow (=output compare 1a)
     * and output compare 1b interrupt */
    _TIMSK_TIMER1 |= _BV(OCIE1A) | _BV(OCIE1B);

    /* set TOP for CTC mode */
    OCR1A = 64000;

    /* load initial delay, trigger an overflow */
    OCR1B = 65000;

    /* reset structures */
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        global_pwm.fade_delay[i] = 1;
        global_pwm.fade_step[i] = 1;
    }

    /* calculate initial timeslots (2 times) */
    update_pwm_timeslots(&global_pwm.current);
    update_pwm_timeslots(&global_pwm.current);

    /* set initial timeslot */
    struct timeslot_t t;
    dequeue_timeslot(&t);
    pwm_next_bitmask = t.mask;

    /* disable fading timers */
    fading.running = 0;
}

/* prepare new timeslots */
void pwm_poll(void)
{
    /* refill timeslots queue */
    while (timeslots_fill() < (PWM_MAX_TIMESLOTS-PWM_CHANNELS-2))
        update_pwm_timeslots(&global_pwm.current);
}

/* update color values for current fading */
void pwm_poll_fading(void)
{
    uint8_t mask = 1;
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        /* check running timers */
        if ( (fading.running & mask) && timer_expired(&fading.timer[i])) {
            update_rgb(i);
            fading.running &= ~mask;
        }

        /* if timer is not running and current != target, start timer */
        if (!(fading.running & mask)
                && global_pwm.current.rgb[i] != global_pwm.target.rgb.rgb[i]
                && global_pwm.fade_delay[i] > 0) {
            timer_set(&fading.timer[i], global_pwm.fade_delay[i]);
            fading.running |= mask;
        }

        /* shift mask */
        mask <<= 1;
    }
}

/** update pwm timeslot table (for target color) */
void update_pwm_timeslots(struct rgb_color_t *target)
{
    static uint8_t sorted[PWM_CHANNELS] = {0, 1, 2};

    /* sort channels according to the current brightness */
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        for (uint8_t j = i+1; j < PWM_CHANNELS; j++) {
            if (target->rgb[sorted[j]] < target->rgb[sorted[i]]) {
                uint8_t temp;

                temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    /* calculate initial bitmask */
    uint8_t initial_bitmask;
#ifdef PWM_INVERTED
    initial_bitmask = PWM_CHANNEL_MASK;
#else
    initial_bitmask = 0;
#endif

    uint8_t chanmask = 1;
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        if (target->rgb[i] > PWM_MIN_BRIGHTNESS) {
#ifdef PWM_INVERTED
            initial_bitmask &= ~chanmask;
#else
            initial_bitmask |= chanmask;
#endif
        }

        chanmask <<= 1;
    }

    /* insert first timeslot */
    enqueue_timeslot(initial_bitmask, 65000);

    /* calculate timeslots and masks */
    uint8_t mask = initial_bitmask;
    uint8_t last_brightness = 0;
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        uint8_t brightness = target->rgb[sorted[i]];

        /* if color is (nearly off) or max, process next color */
        if (brightness <= PWM_MIN_BRIGHTNESS || brightness == 255)
            continue;

        /* check if current timeslot would happen after the middle interrupt */
        if (last_brightness < 181 && brightness >= 181) {
            /* insert (normal) middle interrupt */
            enqueue_timeslot(mask, 65000);
        }

        /* if brightness is new, allocate a new timeslot */
        if (brightness > last_brightness) {

            /* update mask and last_brightness */
            last_brightness = brightness;
            #ifdef PWM_INVERTED
            mask |= _BV(sorted[i]);
            #else
            mask &= ~_BV(sorted[i]);
            #endif

            /* save (normal) timeslot */
            uint16_t top = pgm_read_word(&timeslot_table[target->rgb[sorted[i]] - 1 ]);
            enqueue_timeslot(mask, top);
        } else {
            /* just update mask of last timeslot */
            #ifdef PWM_INVERTED
            mask |= _BV(sorted[i]);
            #else
            mask &= ~_BV(sorted[i]);
            #endif

            update_last_timeslot(mask);
        }
    }

    /* if all interrupts happen before the middle interrupt, insert it here */
    if (last_brightness < 181)
        enqueue_timeslot(mask, 65000);
}

/** fade any channels not already at their target brightness */
void update_rgb(uint8_t c)
{
    /* return if target reached */
    if (global_pwm.current.rgb[c] == global_pwm.target.rgb.rgb[c])
        return;

    /* check direction */
    if (global_pwm.current.rgb[c] < global_pwm.target.rgb.rgb[c]) {
        uint8_t diff = global_pwm.target.rgb.rgb[c] - global_pwm.current.rgb[c];

        if (diff >= global_pwm.fade_step[c])
            global_pwm.current.rgb[c] += global_pwm.fade_step[c];
        else
            global_pwm.current.rgb[c] += diff;

    } else {
        uint8_t diff = global_pwm.current.rgb[c] - global_pwm.target.rgb.rgb[c];

        if (diff >= global_pwm.fade_step[c])
            global_pwm.current.rgb[c] -= global_pwm.fade_step[c];
        else
            global_pwm.current.rgb[c] -= diff;
    }
}

/* timeslot queue handling */
void enqueue_timeslot(uint8_t mask, uint16_t top)
{
    struct timeslot_t *t = &timeslots.slot[timeslots.write];
    t->mask = mask;
    t->top = top;
    timeslots.write = (timeslots.write + 1) % PWM_MAX_TIMESLOTS;
}

void dequeue_timeslot(struct timeslot_t *d)
{
    struct timeslot_t *t = &timeslots.slot[timeslots.read];
    d->mask = t->mask;
    d->top = t->top;
    timeslots.read = (timeslots.read + 1) % PWM_MAX_TIMESLOTS;
}

void update_last_timeslot(uint8_t mask)
{
    uint8_t i;
    if (timeslots.write > 0)
        i = timeslots.write-1;
    else
        i = PWM_MAX_TIMESLOTS-1;

    timeslots.slot[i].mask = mask;
}

uint8_t timeslots_fill(void)
{
    if (timeslots.write >= timeslots.read)
        return timeslots.write - timeslots.read;
    else
        return PWM_MAX_TIMESLOTS - (timeslots.read - timeslots.write);
}

/* convert hsv to rgb color
 * (see http://en.wikipedia.org/wiki/HSL_and_HSV#Conversion_from_HSV_to_RGB )
 * and
 * http://www.enide.net/webcms/uploads/file/projects/powerpicrgb-irda/hsvspace.pdf
 */
void pwm_hsv2rgb(struct dual_color_t *color)
{
    if (color->hsv.saturation == 0) {
        for (uint8_t i = 0; i < PWM_CHANNELS; i++)
            color->rgb.rgb[i] = color->hsv.value;
        return;
    }

    uint16_t h = color->hsv.hue % 360;
    uint8_t s = color->hsv.saturation;
    uint8_t v = color->hsv.value;

    uint16_t f = ((h % 60) * 255 + 30)/60;
    uint16_t p = (v * (255-s)+128)/255;
    uint16_t q = ((v * (255 - (s*f+128)/255))+128)/255;
    uint16_t t = (v * (255 - ((s * (255 - f))/255)))/255;

    uint8_t i = h/60;

    switch (i) {
        case 0:
            color->rgb.rgb[0] = v;
            color->rgb.rgb[1] = t;
            color->rgb.rgb[2] = p;
            break;
        case 1:
            color->rgb.rgb[0] = q;
            color->rgb.rgb[1] = v;
            color->rgb.rgb[2] = p;
            break;
        case 2:
            color->rgb.rgb[0] = p;
            color->rgb.rgb[1] = v;
            color->rgb.rgb[2] = t;
            break;
        case 3:
            color->rgb.rgb[0] = p;
            color->rgb.rgb[1] = q;
            color->rgb.rgb[2] = v;
            break;
        case 4:
            color->rgb.rgb[0] = t;
            color->rgb.rgb[1] = p;
            color->rgb.rgb[2] = v;
            break;
        case 5:
            color->rgb.rgb[0] = v;
            color->rgb.rgb[1] = p;
            color->rgb.rgb[2] = q;
            break;
    }
}

/* convert rgb to hsv color
 * (see http://en.wikipedia.org/wiki/HSL_and_HSV#Conversion_from_RGB_to_HSL_or_HSV )
 * and
 * http://www.enide.net/webcms/uploads/file/projects/powerpicrgb-irda/hsvspace.pdf
 */
void pwm_rgb2hsv(struct dual_color_t *color)
{
    /* search min and max */
    uint8_t max = color->rgb.red;
    uint8_t min = max;

    if (color->rgb.green > max)
        max = color->rgb.green;
    if (color->rgb.blue > max)
        max = color->rgb.blue;

    if (color->rgb.green < min)
        min = color->rgb.green;
    if (color->rgb.blue < min)
        min = color->rgb.blue;

    uint16_t hue = 0;
    uint8_t diff = max - min;
    uint8_t diffh = diff/2;

    /* compute value and saturation */
    color->hsv.value = max;
    color->hsv.saturation = 0;

    if (max > 0)
        color->hsv.saturation = ((255 * diff)+max/2)/max;
    else {
        color->hsv.saturation = 0;
        color->hsv.hue = 0; /* undefined */
        return;
    }

    if (max == min) {
        hue = 0;
    } else if (max == color->rgb.red) {
        hue = (60 * (color->rgb.green - color->rgb.blue) + diffh)/diff + 360;
    } else if (max == color->rgb.green) {
        hue = (60 * (color->rgb.blue - color->rgb.red) + diffh)/diff + 120;
    } else if (max == color->rgb.blue) {
        hue = (60 * (color->rgb.red - color->rgb.green) + diffh)/diff + 240;
    }

    hue = hue % 360;

    color->hsv.hue = hue;
}

/* stop fading, hold current color */
void pwm_stop_fading(void)
{
    memcpy(&global_pwm.target.rgb, &global_pwm.current.rgb, sizeof(struct rgb_color_t));

    /* ignore all timers */
    fading.running = 0;
}


static uint8_t diff_abs(uint8_t a, uint8_t b)
{
    if (a > b)
        return a-b;
    else
        return b-a;
}

static void compute_speed(uint8_t step, uint8_t delay)
{
    /* search for max distance */
    uint8_t max = 0;
    uint8_t dist = 0;

    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        uint8_t d = diff_abs(global_pwm.target.rgb.rgb[i], global_pwm.current.rgb[i]);

        if (d > dist) {
            max = i;
            dist = d;
        }
    }

    /* adjust fading speeds, relative to max distance */
    global_pwm.fade_step[max] = step;
    global_pwm.fade_delay[max] = delay;

    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        if (i == max)
            continue;

        uint8_t d = diff_abs(global_pwm.target.rgb.rgb[i], global_pwm.current.rgb[i]);

        uint8_t ratio = 1;
        if (d > 0)
            ratio = (dist+d/2)/d;

        if (ratio == 0)
            ratio = 1;

        global_pwm.fade_delay[i] = ratio * delay;
        global_pwm.fade_step[i] = step;
    }
}

void pwm_fade_rgb(struct rgb_color_t *color, uint8_t step, uint8_t delay)
{
    /* apply offsets for step and delay */
    step = remote_apply_offset(step, global_remote.offsets.step);
    delay = remote_apply_offset(delay, global_remote.offsets.delay);

    /* set target color */
    for (uint8_t i = 0; i < PWM_CHANNELS; i++)
        global_pwm.target.rgb.rgb[i] = color->rgb[i];

    /* compute correct speed for all channels */
    if (delay == 0)
        delay = 1;
    compute_speed(step, delay);

    /* disable timer */
    fading.running = 0;
}

void pwm_fade_hsv(struct hsv_color_t *color, uint8_t step, uint8_t delay)
{
    /* apply offsets for step and delay */
    step = remote_apply_offset(step, global_remote.offsets.step);
    delay = remote_apply_offset(delay, global_remote.offsets.delay);

    /* convert color */
    memcpy(&global_pwm.target.hsv, color, sizeof(struct hsv_color_t));

    /* apply offsets */
    remote_apply_hsv_offset(&global_pwm.target.hsv);

    /* update rgb color in target */
    pwm_hsv2rgb(&global_pwm.target);

    /* compute correct speed for all channels */
    compute_speed(step, delay);

    /* disable timer */
    fading.running = 0;
}

bool pwm_target_reached(void)
{
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        if (global_pwm.target.rgb.rgb[i] != global_pwm.current.rgb[i])
            return false;
    }

    return true;
}

/* modify current color */
void pwm_modify_rgb(struct rgb_color_offset_t *color, uint8_t step, uint8_t delay)
{
    for (uint8_t i = 0; i < PWM_CHANNELS; i++) {
        int16_t current = global_pwm.target.rgb.rgb[i];
        current += color->rgb[i];

        if (current > 255)
            current = 255;
        if (current < 0)
            current = 0;

        global_pwm.target.rgb.rgb[i] = LO8(current);
    }

    compute_speed(step, delay);

    /* disable timer */
    fading.running = 0;
}

void pwm_modify_hsv(struct hsv_color_offset_t *color, uint8_t step, uint8_t delay)
{
    /* convert current target color from rgb to hsv */
    pwm_rgb2hsv(&global_pwm.target);

    /* apply changes, hue */
    global_pwm.target.hsv.hue += color->hue;

    /* saturation */
    int16_t sat = global_pwm.target.hsv.saturation;
    sat += color->saturation;
    if (sat > 255)
        sat = 255;
    if (sat < 0)
        sat = 0;
    global_pwm.target.hsv.saturation = LO8(sat);

    /* value */
    int16_t val = global_pwm.target.hsv.value;
    val += color->value;
    if (val > 255)
        val = 255;
    if (val < 0)
        val = 0;
    global_pwm.target.hsv.value = LO8(val);

    /* re-convert to rgb */
    pwm_hsv2rgb(&global_pwm.target);

    /* compute correct speed for all channels */
    compute_speed(step, delay);

    /* disable timer */
    fading.running = 0;
}


/** interrupts*/

/** timer1 overflow (=output compare a) interrupt */
ISR(SIG_OUTPUT_COMPARE1A)
{
    /* read next_bitmask */
    uint8_t next_bitmask = pwm_next_bitmask;
    /* output new values */
    P_PORT = (P_PORT & ~(PWM_CHANNEL_MASK)) | (next_bitmask << PWM_SHIFT);
    #if CONFIG_SECONDARY_PWM
    P2_PORT = (P2_PORT & ~(PWM2_CHANNEL_MASK)) | (next_bitmask << PWM2_SHIFT);
    #endif

    /* prepare next interrupt */
    struct timeslot_t t;
    dequeue_timeslot(&t);

    /* if next timeslot would happen too fast or has already happened, just spinlock */
    while (TCNT1 + 100 > t.top)
    {
        /* spin until timer interrupt is near enough */
        while (t.top > TCNT1);

        /* output new values */
        P_PORT = (P_PORT & ~(PWM_CHANNEL_MASK)) | (t.mask << PWM_SHIFT);
        #if CONFIG_SECONDARY_PWM
        P2_PORT = (P2_PORT & ~(PWM2_CHANNEL_MASK)) | (t.mask << PWM2_SHIFT);
        #endif

        /* load next timeslot */
        dequeue_timeslot(&t);
    }

    /* save values for next interrupt */
    OCR1B = t.top;
    pwm_next_bitmask = t.mask;
}

/** timer1 output compare b interrupt */
ISR(SIG_OUTPUT_COMPARE1B, ISR_ALIASOF(SIG_OUTPUT_COMPARE1A));
