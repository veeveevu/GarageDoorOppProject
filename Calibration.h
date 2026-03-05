#ifndef GARAGE_DOOR_CALIBRATION_H
#define GARAGE_DOOR_CALIBRATION_H

#include "LimitSwitch.h"
#include "RotaryEncoder.h"
#include "StepperMotor.h"

class Calibration {
public:
    Calibration(StepperMotor &motor, LimitSwitch &limit_switch, RotaryEncoder &encoder);
    void do_calibration();
private:
    StepperMotor &motor;
    LimitSwitch &limit_switch;
    RotaryEncoder &encoder;
    int total_steps{0};
};

#endif //GARAGE_DOOR_CALIBRATION_H