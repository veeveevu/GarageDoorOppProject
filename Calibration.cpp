#include "Calibration.h"

Calibration::Calibration(StepperMotor& motor, LimitSwitch& limit_switch, RotaryEncoder& encoder)
    : motor(motor), limit_switch(limit_switch), encoder(encoder) {}

void Calibration::do_calibration() {

    printf("Calibration started\n");

    if (limit_switch.is_Open_Switch_pressed()) {
        printf("Start from open\n");

        while (!limit_switch.is_Close_Switch_pressed()) {
            motor.step(MotorDirection::ToClose);
        }
    } else if (limit_switch.is_Close_Switch_pressed()) {
        printf("Start from close\n");

        while (limit_switch.is_Close_Switch_pressed()) {
            motor.step(MotorDirection::ToOpen);
        }

        while (!limit_switch.is_Close_Switch_pressed()) {
            motor.step(MotorDirection::ToClose);
        }
    } else {
        printf("Start from mid\n");
        while (!limit_switch.is_Close_Switch_pressed()) {
            motor.step(MotorDirection::ToClose);
        }
    }

    encoder.flush();
    total_steps = 0;

    while (!limit_switch.is_Open_Switch_pressed()) {
        motor.step(MotorDirection::ToOpen);

        int direction;
        while (encoder.getEvent(direction)) {
            total_steps++;  // only count when encoder actually ticks
        }

    }
    printf("Total steps: %d", total_steps);
}
