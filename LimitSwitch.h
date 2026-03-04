#ifndef GARAGE_DOOR_LIMITSWITCH_H
#define GARAGE_DOOR_LIMITSWITCH_H

#include <cstdio>
#include "pico/stdlib.h"

class LimitSwitch {
public:
    LimitSwitch();
    static bool debounce(int switch_pin);
    bool is_Open_Switch_pressed();
    bool is_Close_Switch_pressed();
private:
    static constexpr int Open_Switch = 14;
    static constexpr int Close_Switch = 15;
    static constexpr int DEBOUNCE_MS = 10;
};

#endif //GARAGE_DOOR_LIMITSWITCH_H