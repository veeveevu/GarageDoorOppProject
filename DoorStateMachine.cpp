#include "DoorStateMachine.h"

DoorStateMachine::DoorStateMachine()
    : state(DoorState::NOT_CALIBRATED), is_calibrated(false)
{
    load_state_from_eeprom();
}

void DoorStateMachine::handle_event(Event event) {
    printf("[STATE] Received event: %d, current state: %d\n", static_cast<int>(event), static_cast<int>(state));

    switch (state) {
    case DoorState::NOT_CALIBRATED:
        if (!is_calibrated || event == Event::SW0_SW2_PRESSED || event == Event::REMOTE_CALIBRATE) {
            state = DoorState::CALIBRATING;
        }
        break;
    case DoorState::CALIBRATING:
        if (event == Event::CALIBRATION_COMPLETE_SUCCESS) {
            is_calibrated = true;
            save_state_to_eeprom();
            state = DoorState::DOOR_CLOSED;
        }
        state = DoorState::ERROR;
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
    char msg[LOG_MAX_STR_LEN];
    snprintf(msg, sizeof(msg), "State:%d,Calib:%d,Dir:%d", static_cast<int>(state), is_calibrated ? 1 : 0, static_cast<int>(last_direction));
    eeprom_log_write(msg);
}

void DoorStateMachine::load_state_from_eeprom() {
    int last_valid_slot = -1;
    for (int i = LOG_MAX_ENTRIES - 1; i >= 0; --i) {
        uint16_t addr = LOG_START_ADDR + i * LOG_ENTRY_SIZE;
        if (is_valid_log_entry(addr)) {
            last_valid_slot = i;
            break;
        }
    }

    if (last_valid_slot == -1) {
        printf("[EEPROM] No valid log entry found, using default state\n");
        state = DoorState::NOT_CALIBRATED;
        is_calibrated = false;
        //last_direction = Direction::None;
        return;
    }

    uint16_t addr = LOG_START_ADDR + last_valid_slot * LOG_ENTRY_SIZE;
    uint8_t buf[LOG_ENTRY_SIZE]{};
    if (!eeprom_read_multi(addr, buf, LOG_ENTRY_SIZE)) {
        printf("[EEPROM] Read failed for slot %d\n", last_valid_slot);
        state = DoorState::NOT_CALIBRATED;
        is_calibrated = false;
        //last_direction = Direction::None;
        return;
    }

    char msg[LOG_MAX_STR_LEN + 1] = {0};
    size_t len = 0;
    while (len < LOG_MAX_STR_LEN && buf[len] != 0) {
        msg[len] = buf[len];
        len++;
    }
    msg[len] = '\0';

    int parsed_state = 0, parsed_calib = 0, parsed_dir = 0;
    if (sscanf(msg, "State:%d,Calib:%d,Dir:%d", &parsed_state, &parsed_calib, &parsed_dir) == 3) {
        state = static_cast<DoorState>(parsed_state);
        is_calibrated = (parsed_calib == 1);
        last_direction = static_cast<Direction>(parsed_dir);
        printf("[EEPROM] Loaded: State=%d, Calib=%d, Dir=%d\n", parsed_state, parsed_calib, parsed_dir);
    } else {
        printf("[EEPROM] Parse failed: '%s'\n", msg);
        state = DoorState::NOT_CALIBRATED;
        is_calibrated = false;
        //last_direction = Direction::None;
    }
}

