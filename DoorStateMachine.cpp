#include "DoorStateMachine.h"

DoorStateMachine::DoorStateMachine()
    : state(DoorState::NOT_CALIBRATED), is_calibrated(false)
{
    //load_state_from_eeprom();
}

void DoorStateMachine::handle_event(Event event) {
    printf("[STATE] Received event: %d, current state: %d\n", static_cast<int>(event), static_cast<int>(state));

    switch (state) {
    case DoorState::NOT_CALIBRATED:
        if (event == Event::SW0_SW2_PRESSED || event == Event::REMOTE_CALIBRATE) {
            state = DoorState::CALIBRATING;
        }
        break;
    case DoorState::CALIBRATING:
        if (event == Event::CALIBRATION_COMPLETE_SUCCESS) {
            is_calibrated = true;
            save_state_to_eeprom();
            state = DoorState::DOOR_CLOSED;
        } else if (event == Event::STUCK_FOUND) {
            printf("[STATE] Calibration failed - going to ERROR\n");
            state = DoorState::ERROR;
        }
        break;
    case DoorState::DOOR_CLOSED:
        if (event == Event::SW1_PRESSED || event == Event::REMOTE_OPEN) {
            state = DoorState::OPENING;
            last_direction = Direction::ToOpen;
        }
        break;
    case DoorState::DOOR_OPENED:
        if (event == Event::SW1_PRESSED || event == Event::REMOTE_CLOSE) {
            state = DoorState::CLOSING;
            last_direction = Direction::ToClose;
        }
        break;
    case DoorState::OPENING:
        // don't know what is the equal remote event?
        if (event == Event::OPEN_SWITCH_TRIGGERED) {
            state = DoorState::DOOR_OPENED;
        }
        if (event == Event::SW1_PRESSED  || event == Event::REMOTE_PAUSE) {
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
        if (event == Event::SW1_PRESSED || event == Event::REMOTE_PAUSE) {
            state = DoorState::STOPPED;
        }
        if (event == Event::STUCK_FOUND) {
            state = DoorState::ERROR;
        }
        break;
    case DoorState::STOPPED:
        if (event == Event::SW1_PRESSED || event == Event::REMOTE_CONTINUE) {
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
        if (event == Event::SW0_SW2_PRESSED || event == Event::REMOTE_CALIBRATE) {
            state = DoorState::CALIBRATING;
        }
        break;

    }
    printf("[STATE] New state: %d\n", static_cast<int>(state));
    publish_mqtt_status();
    save_state_to_eeprom();
}

DoorState DoorStateMachine::get_current_state() {
    return state;
}

bool DoorStateMachine::get_is_calibrated() const {
    return is_calibrated;
}

// Helper để map state cho report
std::string DoorStateMachine::get_door_status_string() const {
    if (state == DoorState::DOOR_OPENED) return "Open";
    if (state == DoorState::DOOR_CLOSED) return "Closed";
    return "In between";  // OPENING, CLOSING, STOPPED
}

std::string DoorStateMachine::get_error_status_string() const {
    if (state == DoorState::ERROR) return "Door stuck";
    return"Normal";
}

std::string DoorStateMachine::get_calibration_status_string() const {
    if (is_calibrated) return "Calibrated";
    return"Not calibrated";
}

void DoorStateMachine::publish_mqtt_status() {
#ifdef USE_MQTT
    if (!client || !client->isConnected()) {
        printf("[MQTT] Client not connected, skip publish status\n");
        return;
    }

    std::string door_str   = get_door_status_string();
    std::string error_str  = get_error_status_string();
    std::string calib_str  = get_calibration_status_string();

    char payload[256];
    int len = snprintf(payload, sizeof(payload),
                       "{\"door\":\"%s\",\"error\":\"%s\",\"calibration\":\"%s\"}",
                       door_str.c_str(), error_str.c_str(), calib_str.c_str());

    if (len < 0 || len >= static_cast<int>(sizeof(payload))) {
        printf("[MQTT] JSON snprintf overflow or error\n");
        return;
    }

    MQTT::Message message;
    message.qos         = MQTT::QOS0;
    message.retained    = true;
    message.dup         = false;
    message.payload     = (void*)payload;
    message.payloadlen  = static_cast<size_t>(len);

    // Publish
    int rc = client->publish("garage/door/status", message);
    if (rc != 0) {
        printf("[MQTT] Publish status failed, rc = %d\n", rc);
    } else {
        printf("[MQTT] Published status: %s\n", payload);
    }
#else
    printf("[DEBUG] Door status: %s | Error: %s | Calib: %s\n",
           get_door_status_string().c_str(),
           get_error_status_string().c_str(),
           get_calibration_status_string().c_str());
#endif
}

void DoorStateMachine::save_state_to_eeprom() {
    uint8_t buf[3];
    buf[0] = static_cast<uint8_t>(state);
    buf[1] = is_calibrated ? 1 : 0;
    buf[2] = static_cast<uint8_t>(last_direction);

    eeprom_write_multi(EEPROM_STATE_ADDR, buf, 3);
    printf("[EEPROM] Saved state: %d, calib: %d, dir: %d\n", buf[0], buf[1], buf[2]);
}

void DoorStateMachine::load_state_from_eeprom() {
    uint8_t buf[3] = {0};
    if (eeprom_read_multi(EEPROM_STATE_ADDR, buf, 3)) {
        bool calib = (buf[1] == 1);
        if (calib && buf[0] <= static_cast<uint8_t>(DoorState::ERROR)) {
            state = static_cast<DoorState>(buf[0]);
            is_calibrated = true;
            last_direction = static_cast<Direction>(buf[2]);
            printf("[EEPROM] Loaded: state=%d, calib=%d, dir=%d\n", buf[0], buf[1], buf[2]);
            return;
        }
    }
    printf("[EEPROM] Reset to NOT_CALIBRATED\n");
    state = DoorState::NOT_CALIBRATED;
    is_calibrated = false;
    last_direction = Direction::None;
}

