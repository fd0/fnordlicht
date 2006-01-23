#ifndef PWM_H
#define PWM_H

/* possible pwm interrupts in a pwm cycle */
#define PWM_MAX_TIMESLOTS (PWM_CHANNELS+1)

/* contains all the data for one color channel */
struct channel_t
/*{{{*/ {
    union {
        /* for adding fade-speed to brightness, and save the remainder */
        uint16_t brightness_and_remainder;

        /* for accessing brightness directly */
        struct {
            uint8_t remainder;
            uint8_t brightness;
        };
    };

    /* desired brightness for this channel */
    uint8_t target_brightness;

    /* fade speed, the msb is added directly to brightness,
     * the lsb is added to the remainder until an overflow happens */
    uint16_t speed;

    /* output mask for switching on the leds for this channel */
    uint8_t mask;

    /* flags for this channel, implemented as a bitvector field */
    struct {
        /* this channel reached has recently reached it's desired target brightness */
        uint8_t target_reached:1;
    } flags;

}; /*}}}*/

struct global_pwm_t {
    /* current channel records */
    struct channel_t channels[3];
};

extern volatile struct global_pwm_t global_pwm;

/* prototypes */
void init_timer1(void);
void init_pwm(void);
void update_pwm_timeslots(void);
void update_brightness(void);

#endif
