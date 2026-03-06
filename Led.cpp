#include "Led.h"

Led::Led(uint led_pin)
    : led(led_pin), state{0}, last_blink_time{0} {
    gpio_init(led);
    gpio_set_dir(led, GPIO_OUT);
}

void Led::turn_led_on() {
    gpio_put(led, 1);
}

void Led::turn_led_off() {
    gpio_put(led, 0);
}

void Led::blink_led() {
    uint64_t interval_us = 250000;
    uint64_t now = time_us_64();

    if (now - last_blink_time >= interval_us) {
        state = !state;
        gpio_put(led, state);
        last_blink_time = now;
    }
}
