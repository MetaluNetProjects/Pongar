#pragma once

#include "fraise.h"

#define printf fraise_printf

class FraiseManager {
private:
    int fraiseoff_switch_pin;
    bool last_pin;
    absolute_time_t pin_timeout = at_the_end_of_time;

    void switch_changed() {
        if(gpio_get(fraiseoff_switch_pin) == 1) { // switch isn't closed: enable Fraise
            printf("l switching Fraise on\n");
            fraise_setup();
        } else { // disable Fraise
            printf("l switching Fraise off\n");
            fraise_unsetup();
        }
    }
public:
    void init(int ap_pin) {
        fraiseoff_switch_pin = ap_pin;
        gpio_init(fraiseoff_switch_pin);
        gpio_set_dir(fraiseoff_switch_pin, GPIO_IN);
        gpio_pull_up(fraiseoff_switch_pin);
        sleep_ms(50); // wait for the pin to settle
        last_pin = gpio_get(fraiseoff_switch_pin);
        pin_timeout = at_the_end_of_time;
        //switch_changed();
    }

    void update() {
        if(last_pin != gpio_get(fraiseoff_switch_pin)) {
            last_pin = gpio_get(fraiseoff_switch_pin);
            pin_timeout = make_timeout_time_ms(1000);
        }
        if(time_reached(pin_timeout)) {
            pin_timeout = at_the_end_of_time;
            printf("l switch_changed\n");
            sleep_ms(100);
            switch_changed();
            last_pin = gpio_get(fraiseoff_switch_pin);
        }
    }
};

