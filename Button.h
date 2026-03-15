#ifndef GARAGE_DOOR_BUTTON_H
#define GARAGE_DOOR_BUTTON_H
#include "pico/stdlib.h"

class Button {
public:
    Button(uint button_pin);
    bool is_pressed();
    bool is_held() const;

private:
    uint button;
    absolute_time_t last_time = nil_time;
    bool last_state = false;
    const uint32_t debounce_us = 50000;

};
#endif //GARAGE_DOOR_BUTTON_H