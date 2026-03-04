#ifndef GARAGE_DOOR_ROTARYENCODER_H
#define GARAGE_DOOR_ROTARYENCODER_H

#include <cstdio>
#include "pico/stdlib.h"
#include "pico/util/queue.h"

class RotaryEncoder {
public:
    RotaryEncoder();
    static void encoder_handler(unsigned gpio, uint32_t event_mask);
    static queue_t rotary_events;
private:
    static constexpr int Rot_A = 27;
    static constexpr int Rot_B = 28;
};

#endif //GARAGE_DOOR_ROTARYENCODER_H
