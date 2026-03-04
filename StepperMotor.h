#ifndef GARAGE_DOOR_STEPPERMOTOR_H
#define GARAGE_DOOR_STEPPERMOTOR_H
#include "pico/stdlib.h"
#include "cstdio"

enum class MotorDirection {
    Forward, //close -> open
    Backward //open -> close
};

class StepperMotor {
public:
    StepperMotor();

    enum class MotorDirection {
        Forward, //close -> open
        Backward //open -> close
    };

    void step(MotorDirection direction);
    void run_steps(int steps, MotorDirection direction);

private:
    static constexpr int STEP_DELAY_US = 1500;
    static constexpr int motor_pins[4] = {2, 3, 6, 13}; // ~ IN1, IN2, IN3, IN4
    static constexpr int step_sequence[8][4]= {
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,1,1,0},
        {0,0,1,0},
        {0,0,1,1},
        {0,0,0,1},
        {1,0,0,1}
    };

    int current_step = 0;
};

#endif //GARAGE_DOOR_STEPPERMOTOR_H