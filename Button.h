#ifndef GARAGE_DOOR_BUTTON_H
#define GARAGE_DOOR_BUTTON_H
#include "pico/stdlib.h"

/*
#define SW_0 9
#define SW_1 8
#define SW_2 7
*/

class Button {
public:
    Button(uint button_pin);
    bool is_pressed();

private:
    uint button;
    absolute_time_t last_time = nil_time;
    bool last_state = false;
    bool initialized = false;
    const uint32_t debounce_us = 30000;

};
#endif //GARAGE_DOOR_BUTTON_H