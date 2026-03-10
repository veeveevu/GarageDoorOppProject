#ifndef GARAGE_DOOR_STEPPERMOTOR_H
#define GARAGE_DOOR_STEPPERMOTOR_H
#include "pico/stdlib.h"
#include "cstdio"

//{2, 3, 6, 13} // ~ IN1, IN2, IN3, IN4

enum class Direction {
    ToOpen, //close -> open
    ToClose, //open -> close
    None
};

class StepperMotor {
public:
    StepperMotor(uint in1_pin, uint in2_pin, uint in3_pin, uint in4_pin);

    void step(Direction direction);
    void run_steps(int steps, Direction direction);

private:
    static constexpr int STEP_DELAY_US = 1100;
    uint motor_pins[4];
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