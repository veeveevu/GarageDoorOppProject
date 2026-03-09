#ifndef GARAGE_DOOR_CALIBRATION_H
#define GARAGE_DOOR_CALIBRATION_H

#include "LimitSwitch.h"
#include "RotaryEncoder.h"
#include "StepperMotor.h"

class Calibration {
public:
    Calibration(StepperMotor &motor, LimitSwitch &limit_switch, RotaryEncoder &encoder);
    void do_calibration();
    int get_encoder_counter() const;
    int get_motor_counter() const;

private:
    StepperMotor &motor;
    LimitSwitch &limit_switch;
    RotaryEncoder &encoder;
    int encoder_counter{0};
    int motor_counter{0};
};

#endif //GARAGE_DOOR_CALIBRATION_H