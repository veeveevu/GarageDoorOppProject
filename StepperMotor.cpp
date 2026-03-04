#include "StepperMotor.h"

StepperMotor::StepperMotor() {
    for (int motor_pin : motor_pins) {
        gpio_init(motor_pin);
        gpio_set_dir(motor_pin, GPIO_OUT);
    }
}

void StepperMotor::step(MotorDirection direction) {

    if (direction == MotorDirection::Forward) {
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
    int steps_per_revolution = 2048; //fake num

    int steps_to_run = steps * (steps_per_revolution / 8);

    for (int i = 0; i < steps_to_run; i ++) {
        step(direction);
        //sleep_ms(STEP_DELAY_MS);
    }
    printf("Done running!\n");

}