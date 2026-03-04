#include "RotaryEncoder.h"

queue_t RotaryEncoder::rotary_events;


RotaryEncoder::RotaryEncoder() {

    gpio_init(Rot_A);
    gpio_set_dir(Rot_A, GPIO_IN);
    gpio_disable_pulls(Rot_A);

    gpio_init(Rot_B);
    gpio_set_dir(Rot_B, GPIO_IN);
    gpio_disable_pulls(Rot_B);

    //initialize queue
    queue_init(&rotary_events, sizeof(int), 10);

    //set interrupt (only for Rot_A!!!)
    gpio_set_irq_enabled_with_callback(Rot_A, GPIO_IRQ_EDGE_RISE, true, RotaryEncoder::encoder_handler);
}

void RotaryEncoder::encoder_handler(unsigned gpio, uint32_t event_mask) {
    if (gpio == Rot_A) {
        int direction;
        if (gpio_get(Rot_B) == 0) {
            direction = 1; //turn right
            printf("Turn right");
        }
        else {
            direction = -1; //turn left
            printf("Turn right");
        }
        queue_try_add(&rotary_events, &direction);
    }
}

