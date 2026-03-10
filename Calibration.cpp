#include "Calibration.h"

Calibration::Calibration(StepperMotor& motor, LimitSwitch& limit_switch, RotaryEncoder& encoder)
    : motor(motor), limit_switch(limit_switch), encoder(encoder) {}

bool Calibration::do_calibration() {

    printf("[CALIB] Calibration started...\n");

    if (limit_switch.is_Open_Switch_held()) {
        printf("[CALIB] Start from OPEN\n");
        int safety = 0;

        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK when first move to close!\n");
                return false;

            }
        }
    } else if (limit_switch.is_Close_Switch_held()) {
        printf("[CALIB] Start from CLOSE\n");
        int safety = 0;
        while (limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToOpen);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK found.\n");
                return false;
            }
        }

        safety = 0;
        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK found.\n");
                return false;
            }
        }
    } else {
        printf("[CALIB] Start from MID\n");
        int safety = 0;
        while (!limit_switch.is_Close_Switch_held()) {
            motor.step(Direction::ToClose);
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK found.\n");
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
            while (encoder.get_event(direction)) {
                ++total_encoder_counter;
            }
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK during trip 1!\n");
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
            while (encoder.get_event(direction)) {
                ++total_encoder_counter;
            }
            if (++safety > MAX_STEPS_SAFETY) {
                printf("[CALIB] STUCK during trip 2!\n");
                return false;
            }
        }
    }

    encoder_counter = total_encoder_counter / 2;
    motor_counter = total_motor_counter / 2;

    //printf("Encoder steps: %d\n", encoder_counter);
    //printf("Motor steps: %d\n", motor_counter);
    return true;
}

int Calibration::get_encoder_counter() const {
    return encoder_counter;
}

int Calibration::get_motor_counter() const {
    return motor_counter;
}


