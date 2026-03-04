#include "RotaryEncoder.h"

RotaryEncoder* RotaryEncoder::instance = nullptr;

RotaryEncoder::RotaryEncoder(uint Rot_A_pin, uint Rot_B_pin)
    : Rot_A(Rot_A_pin), Rot_B(Rot_B_pin) {
    instance = this;
    gpio_init(Rot_A);
    gpio_set_dir(Rot_A, GPIO_IN);
    gpio_disable_pulls(Rot_A);

    gpio_init(Rot_B);
    gpio_set_dir(Rot_B, GPIO_IN);
    gpio_disable_pulls(Rot_B);

    //initialize queue
    queue_init(&rotary_events, sizeof(int), 10);

    //set interrupt (only for Rot_A!!!)
    gpio_set_irq_enabled_with_callback(Rot_A, GPIO_IRQ_EDGE_RISE, true, encoder_handler);
}

void RotaryEncoder::encoder_handler(uint gpio, uint32_t event_mask) {
    if (gpio == instance->Rot_A) {
        int direction;
        if (gpio_get(instance->Rot_B) == 0) {
            direction = 1; //turn right
            printf("To close");
        }
        else {
            direction = -1; //turn left
            printf("To open");
        }
        queue_try_add(&instance->rotary_events, &direction);
    }
}
