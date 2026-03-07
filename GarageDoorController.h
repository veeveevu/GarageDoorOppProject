#ifndef GARAGE_DOOR_GARAGEDOORCONTROLLER_H
#define GARAGE_DOOR_GARAGEDOORCONTROLLER_H

#include "Button.h"
#include "EEPROM.h"
#include "Led.h"
#include "LimitSwitch.h"
#include "MQTTCom.h"
#include "RotaryEncoder.h"
#include "StepperMotor.h"
#include "DoorStateMachine.h"

class GarageDoorController {
public:
    GarageDoorController();
    void run();  // Main loop

private:
    DoorStateMachine m_stateMachine;
    StepperMotor m_motor;
    RotaryEncoder m_encoder;
    LimitSwitche m_limits;

    // Button states for debounce
    bool last_sw0 = true, last_sw1 = true, last_sw2 = true;
    absolute_time_t last_debounce = 0;
    const uint debounce_ms = 50;

    // Calibration vars
    int total_steps = 0;  // From calib, to stop before body
    int current_pos = 0;  // Track position via encoder

    // Stuck detection
    absolute_time_t last_encoder_time = 0;
    const uint stuck_timeout_ms = 500;

    void checkButtons();
    void checkLimits();
    void checkStuck();
    void reactToState();
    void updateLEDs();
    void performCalibration();
    void saveState();
    void loadState();
};

#endif //GARAGE_DOOR_GARAGEDOORCONTROLLER_H