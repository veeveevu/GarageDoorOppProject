#include "DoorStateMachine.h"

DoorStateMachine::DoorStateMachine()
    :state(DoorState::NOT_CALIBRATED)
{}

void DoorStateMachine::handle_event(Event event) {
    switch (state) {
    case DoorState::NOT_CALIBRATED:
        if (event == Event::SW0_SW2_PRESSED) {
            state = DoorState::CALIBRATING;
        }
        break;
    case DoorState::CALIBRATING:
        //cant think of how to move next state yet
        state = DoorState::DOOR_CLOSED;
        break;
    case DoorState::DOOR_CLOSED:
        if (event == Event::SW1_PRESSED) {
            state = DoorState::OPENING;
            last_direction = Direction::ToOpen;
        }
        break;
    case DoorState::DOOR_OPENED:
        if (event == Event::SW1_PRESSED) {
            state = DoorState::CLOSING;
            last_direction = Direction::ToClose;
        }
        break;
    case DoorState::OPENING:
        if (event == Event::OPEN_SWITCH_TRIGGERED) {
            state = DoorState::DOOR_OPENED;
        }
        if (event == Event::SW1_PRESSED) {
            state = DoorState::STOPPED;
        }
        if (event == Event::STUCK_FOUND) {
            state = DoorState::ERROR;
        }
        break;
    case DoorState::CLOSING:
        if (event == Event::CLOSE_SWITCH_TRIGGERED) {
            state = DoorState::DOOR_CLOSED;
        }
        if (event == Event::SW1_PRESSED) {
            state = DoorState::STOPPED;
        }
        if (event == Event::STUCK_FOUND) {
            state = DoorState::ERROR;
        }
        break;
    case DoorState::STOPPED:
        if (event == Event::SW1_PRESSED) {
            if (last_direction == Direction::ToOpen) {
                state = DoorState::CLOSING;
                last_direction = Direction::ToClose;
            } else {
                state = DoorState::OPENING;
                last_direction = Direction::ToOpen;
            }
        }
        break;
    case DoorState::ERROR:
        if (event == Event::SW0_SW2_PRESSED) {
            state = DoorState::CALIBRATING;
        }
        break;

    }

}

DoorState DoorStateMachine::get_current_state() {
    return state;
}

