#include "StepperMotor.h"

StepperMotor::StepperMotor(uint in1_pin, uint in2_pin, uint in3_pin, uint in4_pin)
    :motor_pins{in1_pin, in2_pin, in3_pin, in4_pin}{

    for (uint motor_pin : motor_pins) {
        gpio_init(motor_pin);
        gpio_set_dir(motor_pin, GPIO_OUT);
    }
}

void StepperMotor::step(MotorDirection direction) {

    if (direction == MotorDirection::ToOpen) {
        current_step = (current_step + 1) % 8;
    } else {
        current_step = (current_step - 1 + 8) % 8;
    }

    for (int i = 0; i < 4; i++) {
        gpio_put(motor_pins[i], step_sequence[current_step][i]);
    }

    sleep_us(STEP_DELAY_US);
}

void StepperMotor::run_steps(int steps, MotorDirection direction) {

    for (int i = 0; i < steps; i ++) {
        step(direction);
        //sleep_ms(STEP_DELAY_MS);
    }
    printf("Done running!\n");

}