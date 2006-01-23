/* global flag(=bit) structure */
struct flags_t {
    /* set by pwm interrupt after burst, signals the beginning of a new pwm
     * cycle to the main loop. */
    uint8_t new_cycle:1;
    /* set by pwm interrupt after last interrupt in the current cycle, signals
     * the main loop to rebuild the pwm timslot table */
    uint8_t last_pulse:1;
};

struct global_t {
    struct flags_t flags;
};

extern volatile struct global_t global;
