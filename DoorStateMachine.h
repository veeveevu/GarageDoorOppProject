#ifndef GARAGE_DOOR_DOORSTATE_H
#define GARAGE_DOOR_DOORSTATE_H

#include "Calibration.h"
#include "EEPROM.h"
#include "MQTTCom.h"

extern MQTT::Client<IPStack, Countdown>* client;
extern bool client_is_connected();

constexpr uint16_t EEPROM_STATE_ADDR = 32752;  // 3 bytes: 32752-32754
constexpr uint16_t EEPROM_STEPS_ADDR = 32756;  // 8 bytes: 32756-32763
//32764-32767 = spare gap before chip end

//state for state machine
enum class DoorState {
    NOT_CALIBRATED, CALIBRATING, DOOR_CLOSED, DOOR_OPENED, CLOSING, OPENING, STOPPED, ERROR
};

enum class Error {
    NORMAL, STUCK
};

enum class Event {
    SW0_SW2_PRESSED,
    SW1_PRESSED,
    OPEN_SWITCH_TRIGGERED,
    CLOSE_SWITCH_TRIGGERED,
    STUCK_FOUND,
    CALIBRATION_COMPLETE_SUCCESS,
    REMOTE_CALIBRATE,
    REMOTE_OPEN,
    REMOTE_CLOSE,
    REMOTE_PAUSE,
    REMOTE_CONTINUE,
    FINISH_MOVING,
    //them mqtt message event
};

class DoorStateMachine {
public:
    DoorStateMachine();
    void handle_event(Event event);
    DoorState get_current_state();
    bool get_is_calibrated() const;
    std::string get_door_status_string() const;
    std::string get_error_status_string() const;
    std::string get_calibration_status_string() const;
    void publish_mqtt_status();
    void save_state_to_eeprom();
    void load_state_from_eeprom();

private:
    DoorState state;
    Direction last_direction{};
    bool is_calibrated;
    bool stuck_detected = false;
};

#endif //GARAGE_DOOR_DOORSTATE_H