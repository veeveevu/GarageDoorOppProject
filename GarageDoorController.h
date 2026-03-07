#ifndef GARAGE_DOOR_GARAGEDOORCONTROLLER_H
#define GARAGE_DOOR_GARAGEDOORCONTROLLER_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/util/queue.h"
#include "Button.h"
#include "EEPROM.h"
#include "Led.h"
#include "LimitSwitch.h"
#include "MQTTCom.h"
#include "RotaryEncoder.h"
#include "StepperMotor.h"
#include "DoorStateMachine.h"

constexpr uint MOTOR_IN1 = 2;
constexpr uint MOTOR_IN2 = 3;
constexpr uint MOTOR_IN3 = 6;
constexpr uint MOTOR_IN4 = 13;

constexpr uint ENC_A_PIN = 4;
constexpr uint ENC_B_PIN = 5;

constexpr uint LIMIT_OPEN_PIN  = 14;
constexpr uint LIMIT_CLOSE_PIN = 15;

constexpr uint SW0_PIN = 9;
constexpr uint SW1_PIN = 8;
constexpr uint SW2_PIN = 7;

constexpr uint LED_OPEN_PIN  = 20;
constexpr uint LED_CLOSE_PIN = 21;
constexpr uint LED_ERROR_PIN = 22;

class GarageDoorController {
public:
    GarageDoorController();
    void run();

private:
    DoorStateMachine state_machine;

    StepperMotor motor{MOTOR_IN1, MOTOR_IN2, MOTOR_IN3, MOTOR_IN4};
    RotaryEncoder encoder{ENC_A_PIN, ENC_B_PIN};
    LimitSwitch limits{LIMIT_OPEN_PIN, LIMIT_CLOSE_PIN};

    Button sw0{SW0_PIN};
    Button sw1{SW1_PIN};
    Button sw2{SW2_PIN};

    Led led_open{LED_OPEN_PIN};
    Led led_close{LED_CLOSE_PIN};
    Led led_error{LED_ERROR_PIN};

    absolute_time_t last_debounce = nil_time;
    absolute_time_t last_encoder_change = nil_time;
    const uint32_t debounce_us = 30000;
    const uint32_t stuck_timeout_us = 800000;

    int total_steps = 0;
    int current_pos = 0;

    void check_buttons();
    void check_limits_and_encoder();
    void check_stuck();
    void react_to_state();
    void perform_calibration();
    void update_leds();
    void load_state();
    void save_state();
};

#endif //GARAGE_DOOR_GARAGEDOORCONTROLLER_H