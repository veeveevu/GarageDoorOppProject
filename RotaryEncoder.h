#ifndef GARAGE_DOOR_ROTARYENCODER_H
#define GARAGE_DOOR_ROTARYENCODER_H

#include <cstdio>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

class RotaryEncoder {
public:
    RotaryEncoder(uint Rot_A_pin, uint Rot_B_pin);
    bool get_event(int &direction);
    void flush();
private:
    queue_t rotary_events{};
    uint Rot_A;
    uint Rot_B;

    static void encoder_handler(uint gpio, uint32_t event_mask);
    static RotaryEncoder* instance;
};

#endif //GARAGE_DOOR_ROTARYENCODER_H
