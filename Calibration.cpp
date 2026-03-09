#include "Calibration.h"

Calibration::Calibration(StepperMotor& motor, LimitSwitch& limit_switch, RotaryEncoder& encoder)
    : motor(motor), limit_switch(limit_switch), encoder(encoder) {}

bool Calibration::do_calibration() {

    printf("[CALIB] Calibration started\n");

    if (limit_switch.is_Open_Switch_held()) {
        printf("[CALIB] Start from open\n");
        int safety = 0;

        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK when first move to close!\n");
                return false;

            }
        }
    } else if (limit_switch.is_Close_Switch_held()) {
        printf("[CALIB] Start from close\n");
        int safety = 0;
        while (limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToOpen);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK!\n");
                return false;
            }
        }

        safety = 0;
        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK returning to close!\n");
                return false;
            }
        }
    } else {
        printf("[CALIB] Start from mid\n");
        int safety = 0;
        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK driving to close from mid!\n");
                return false;
            }
        }
    }

    encoder.flush();

    int total_encoder_counter{0};
    int total_motor_counter{0};

    //trip 1
    {
        int safety = 0;
        while (!limit_switch.is_Open_Switch_held()) {
            motor.step(Direction::ToOpen);
            ++total_motor_counter;

            int direction;
            while (encoder.getEvent(direction)) {
                ++total_encoder_counter;  // only count when encoder actually ticks
            }
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK during trip 1 (close→open)!\n");
                return false;
            }
        }
    }

    //trip 2
    {
        int safety = 0;
        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            ++total_motor_counter;

            int direction;
            while (encoder.getEvent(direction)) {
                ++total_encoder_counter;  // only count when encoder actually ticks
            }
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK during trip 2 (open→close)!\n");
                return false;
            }
        }
    }

    encoder_counter = total_encoder_counter / 2;
    motor_counter = total_motor_counter / 2;

    printf("Encoder steps: %d\n", encoder_counter);
    printf("Motor steps: %d\n", motor_counter);
    return true;
}

int Calibration::get_encoder_counter() const {
    return encoder_counter;
}

int Calibration::get_motor_counter() const {
    return motor_counter;
}


