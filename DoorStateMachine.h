#ifndef GARAGE_DOOR_DOORSTATE_H
#define GARAGE_DOOR_DOORSTATE_H

#include "Calibration.h"

//state for state machine
enum class DoorState {
    NOT_CALIBRATED, CALIBRATING, DOOR_CLOSED, DOOR_OPENED, CLOSING, OPENING,  STOPPED, ERROR
};

enum class Error {
    NORMAL, STUCK
};

enum class Event {
    SW0_SW2_PRESSED,
    SW1_PRESSED,
    OPEN_SWITCH_TRIGGERED,
    CLOSE_SWITCH_TRIGGERED,
    STUCK_FOUND
    //them mqtt message event
};

class DoorStateMachine {
public:
    DoorStateMachine();
    void handle_event(Event event);
    DoorState get_current_state();
private:
    DoorState state;
    Direction last_direction{};
};

#endif //GARAGE_DOOR_DOORSTATE_H