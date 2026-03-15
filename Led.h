#ifndef GARAGE_DOOR_LED_H
#define GARAGE_DOOR_LED_H

#include "cstdio"
#include "pico/stdlib.h"

class Led {
public:
    Led(uint led_pin);
    void turn_led_on();
    void turn_led_off();
    void blink_led();
private:
    uint led;
    bool state;
    uint64_t last_blink_time;
};

#endif //GARAGE_DOOR_LED_H