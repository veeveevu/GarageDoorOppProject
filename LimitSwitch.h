#ifndef GARAGE_DOOR_LIMITSWITCH_H
#define GARAGE_DOOR_LIMITSWITCH_H

#include <cstdio>
#include "pico/stdlib.h"

//Open_Switch = 14
//Close_Switch = 15;

class LimitSwitch {
public:
    LimitSwitch(uint open_switch_pin, uint close_switch_pin);

    bool is_Open_Switch_pressed();
    bool is_Close_Switch_pressed();
private:
    bool open_was_pressed = false;
    bool close_was_pressed = false;
    uint Open_Switch;
    uint Close_Switch;
    static constexpr int DEBOUNCE_MS = 10;
    bool debounce(uint switch_pin, const char* switch_name);
};

#endif //GARAGE_DOOR_LIMITSWITCH_H