#include "Calibration.h"

Calibration::Calibration(StepperMotor& motor, LimitSwitch& limit_switch, RotaryEncoder& encoder)
    : motor(motor), limit_switch(limit_switch), encoder(encoder) {}

void Calibration::do_calibration() {

    printf("Calibration started\n");

    if (limit_switch.is_Open_Switch_pressed()) {
        printf("Start from open\n");

        while (!limit_switch.is_Close_Switch_pressed()) {
            motor.step(Direction::ToClose);
        }
    } else if (limit_switch.is_Close_Switch_pressed()) {
        printf("Start from close\n");

        while (limit_switch.is_Close_Switch_pressed()) {
            motor.step(Direction::ToOpen);
        }

        while (!limit_switch.is_Close_Switch_pressed()) {
            motor.step(Direction::ToClose);
        }
    } else {
        printf("Start from mid\n");
        while (!limit_switch.is_Close_Switch_pressed()) {
            motor.step(Direction::ToClose);
        }
    }

    encoder.flush();

    int total_encoder_counter{0};
    int total_motor_counter{0};

    //trip 1
    while (!limit_switch.is_Open_Switch_pressed()) {
        motor.step(Direction::ToOpen);
        ++total_motor_counter;

        int direction;
        while (encoder.getEvent(direction)) {
            ++total_encoder_counter;  // only count when encoder actually ticks
        }
    }

    //trip 2
    while (!limit_switch.is_Close_Switch_pressed()) {
        motor.step(Direction::ToClose);
        ++total_motor_counter;

        int direction;
        while (encoder.getEvent(direction)) {
            ++total_encoder_counter;  // only count when encoder actually ticks
        }
    }

    encoder_counter = total_encoder_counter / 2;
    motor_counter = total_motor_counter / 2;

    printf("Encoder steps: %d\n", encoder_counter);
    printf("Motor steps: %d\n", motor_counter);
}

int Calibration::get_encoder_counter() const {
    return encoder_counter;
}

int Calibration::get_motor_counter() const {
    return motor_counter;
}


