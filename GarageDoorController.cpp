#include "GarageDoorController.h"
#include <cstdio>
#include <string>

GarageDoorController* GarageDoorController::instance = nullptr;

GarageDoorController::GarageDoorController()
    : calibration_machine(motor, limits, encoder) {
    printf("[CONSTR] Start constructor...\n");

    printf("[CONSTR] Before mqtt_init()\n");
    mqtt_init();
    printf("[CONSTR] After mqtt_init()\n");

    printf("[CONSTR] Before eeprom_log_init()\n");
    eeprom_log_init();
    printf("[CONSTR] After eeprom_log_init()\n");

    printf("[CONSTR] Before load_state()\n");
    load_state();
    printf("[CONSTR] After load_state()\n");

    instance = this;
    printf("[CONSTR] Constructor done!\n");
}

void GarageDoorController::run() {
    //printf("[RUN] Enter run loop\n");
    check_buttons();
    check_limits_and_encoder();
    if (state_machine.get_current_state() == DoorState::CALIBRATING) {
        perform_calibration();
    } else {
        check_stuck();
        react_to_state();
    }
    update_leds();
    //mqtt_loop();
    sleep_us(5000);
}

void GarageDoorController::check_buttons() {

    /* debounce in button class
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_debounce, now) < debounce_us) {
        //printf("[CHECK_BTN DEBUG] Debounce skip\n");
        return;
    }
    */
    bool sw0_pressed = sw0.is_pressed();
    bool sw1_pressed = sw1.is_pressed();
    bool sw2_pressed = sw2.is_pressed();

    if (sw0_pressed && sw2_pressed) {
        printf("[BTN] SW0 + SW2 pressed → Calibration!\n");
        state_machine.handle_event(Event::SW0_SW2_PRESSED);
        return;
    }
    if (sw1_pressed) {
        printf("[BTN] SW1 pressed!\n");
        state_machine.handle_event(Event::SW1_PRESSED);
    }
    else if (sw0_pressed) {
        printf("[BTN] SW0 pressed alone\n");
    }
    else if (sw2_pressed) {
        printf("[BTN] SW2 pressed alone\n");
    }

    //last_debounce = now;
}

void GarageDoorController::check_limits_and_encoder() {
    if (limits.is_Open_Switch_pressed()) {
        state_machine.handle_event(Event::OPEN_SWITCH_TRIGGERED);
    }
    if (limits.is_Close_Switch_pressed()) {
        state_machine.handle_event(Event::CLOSE_SWITCH_TRIGGERED);
    }

    int dir;
    if (encoder.getEvent(dir)) {
        current_pos += dir;
        last_encoder_change = get_absolute_time();
    }
}

void GarageDoorController::check_stuck() {
    auto st = state_machine.get_current_state();
    if (st == DoorState::OPENING || st == DoorState::CLOSING) {
        absolute_time_t now = get_absolute_time();
        int64_t time_since_movement = absolute_time_diff_us(last_encoder_change, now);

        if (time_since_movement > stuck_timeout_us) {
            state_machine.handle_event(Event::STUCK_FOUND);
            printf("[STUCK] No movement detected for %lld us!\n", time_since_movement);
        }
    }
}

void GarageDoorController::react_to_state() {
    auto st = state_machine.get_current_state();
    static DoorState last_state = DoorState::NOT_CALIBRATED;

    // Reset encoder timer when entering movement states
    if ((st == DoorState::OPENING || st == DoorState::CLOSING) &&
        (last_state != st)) {
        last_encoder_change = get_absolute_time();
        printf("[REACT] Started movement, reset stuck timer\n");
        }
    last_state = st;

    switch (st) {
    case DoorState::CALIBRATING:
        perform_calibration();
        break;

    case DoorState::OPENING:
        motor.step(Direction::ToOpen);
        break;

    case DoorState::CLOSING:
        motor.step(Direction::ToClose);
        break;

    case DoorState::STOPPED:
        break;

    case DoorState::ERROR:
        break;

    default:
        break;
    }
}

/*
void GarageDoorController::perform_calibration() {
    printf("[CALIB] Starting calibration...\n");

    // Run to open limit
    int steps_open = 0;
    while (!limits.is_Open_Switch_pressed()) {
        motor.step(Direction::ToOpen);
        int dir;
        if (encoder.getEvent(dir)) steps_open += dir;
        sleep_us(1000);
    }

    // Run back to close limit
    int steps_close = 0;
    while (!limits.is_Close_Switch_pressed()) {
        motor.step(Direction::ToClose);
        int dir;
        if (encoder.getEvent(dir)) steps_close += dir;
        sleep_us(1000);
    }

    total_steps = steps_open - steps_close;
    current_pos = 0;
    printf("[CALIB] Total steps: %d\n", total_steps);

    state_machine.handle_event(Event::CALIBRATION_COMPLETE_SUCCESS);
    save_state();
}
*/

void GarageDoorController::perform_calibration() {
    if (calibration_started) {
        return;
    }

    calibration_started = true;

    printf("[CALIB] Starting calibration...\n");
    printf("[CALIB] Current state before: %d\n", static_cast<int>(state_machine.get_current_state()));
    bool success = calibration_machine.do_calibration();

    total_motor_steps = calibration_machine.get_motor_counter();
    total_encoder_turns = calibration_machine.get_encoder_counter();
    current_pos = 0;

if (success) {
    printf("[CALIB] Calibration succeeded!\n");

    printf("[CALIB] Total motor steps: %d\n", total_motor_steps);
    printf("[CALIB] Total encode turns: %d\n", total_encoder_turns);
    printf("[CALIB] Current state before event: %d\n", static_cast<int>(state_machine.get_current_state()));

    state_machine.handle_event(Event::CALIBRATION_COMPLETE_SUCCESS);
    printf("[CALIB] Current state after event: %d\n", static_cast<int>(state_machine.get_current_state()));

    save_state();
} else {
    printf("[CALIB] Failed — motor stuck!\n");
    state_machine.handle_event(Event::STUCK_FOUND);
}
    calibration_started = false;
}

void GarageDoorController::update_leds() {
    auto st = state_machine.get_current_state();

    if (st == DoorState::DOOR_OPENED) {
        led_open.turn_led_on();
        led_close.turn_led_off();
    }
    else if (st == DoorState::DOOR_CLOSED) {
        led_close.turn_led_on();
        led_open.turn_led_off();
    }
    else {
        led_open.turn_led_off();
        led_close.turn_led_off();
    }

    if (st == DoorState::ERROR) {
        led_error.blink_led();
    }
    else {
        led_error.turn_led_off();
    }
}

void GarageDoorController::load_state() {
    state_machine.load_state_from_eeprom();
}

void GarageDoorController::save_state() {
    state_machine.save_state_to_eeprom();
}

void GarageDoorController::messageArrived(MQTT::MessageData& md) {
    printf("[MQTT] messageArrived called!\n");
    if (!instance) {
        printf("[MQTT] Controller instance not set!\n");
        return;
    }

    MQTT::Message& message = md.message;

    char payload[message.payloadlen + 1];
    memcpy(payload, message.payload, message.payloadlen);
    payload[message.payloadlen] = '\0';

    printf("[MQTT DEBUG] Received message: '%s'\n", payload);

    std::string cmd(payload);
    std::string result = "Success";

    DoorState current_state = instance->state_machine.get_current_state();
    bool is_calib = instance->state_machine.get_is_calibrated();

    if (cmd == "open" || cmd == "OPEN") {
        if (!is_calib) {
            result = "Error: Not calibrated";
        }
        else if (current_state == DoorState::DOOR_OPENED) {
            result = "Already open";
        }
        else if (current_state == DoorState::OPENING) {
            result = "Already opening";
        }
        else {
            instance->state_machine.handle_event(Event::REMOTE_OPEN);
            printf("[MQTT DEBUG] Triggered REMOTE_OPEN\n");
        }
    }
    else if (cmd == "close" || cmd == "CLOSE") {
        if (!is_calib) {
            result = "Error: Not calibrated";
        }
        else if (current_state == DoorState::DOOR_CLOSED) {
            result = "Already closed";
        }
        else if (current_state == DoorState::CLOSING) {
            result = "Already closing";
        }
        else {
            instance->state_machine.handle_event(Event::REMOTE_CLOSE);
            printf("[MQTT DEBUG] Triggered REMOTE_OPEN\n");
        }
    }
    else if (cmd == "pause" || cmd == "PAUSE" || cmd == "stop" || cmd == "STOP") {
        if (current_state == DoorState::OPENING || current_state == DoorState::CLOSING) {
            instance->state_machine.handle_event(Event::REMOTE_PAUSE);
            printf("[MQTT DEBUG] Triggered REMOTE_PAUSE\n");
        }
        else {
            result = "Not moving, ignore pause";
        }
    }
    else if (cmd == "continue" || cmd == "CONTINUE") {
        if (current_state == DoorState::STOPPED) {
            instance->state_machine.handle_event(Event::REMOTE_CONTINUE);
            printf("[MQTT DEBUG] Triggered REMOTE_CONTINUE\n");
        }
        else {
            result = "Not stopped, ignore continue";
        }
    }
    else if (cmd == "calibrate" || cmd == "CALIBRATE") {
        instance->state_machine.handle_event(Event::REMOTE_CALIBRATE);
        printf("[MQTT DEBUG] Triggered REMOTE_CALIBRATE\n");
    }
    else {
        result = "Error: Unknown command";
    }

    printf("[MQTT DEBUG] Command result: %s\n", result.c_str());

    char resp[128];
    snprintf(resp, sizeof(resp), "{\"command\":\"%s\",\"result\":\"%s\"}", payload, result.c_str());
    printf("[MQTT DEBUG] Preparing response: %s\n", resp);

    MQTT::Message resp_msg;
    resp_msg.qos = MQTT::QOS0;
    resp_msg.retained = false;
    resp_msg.payload = (void*)resp;
    resp_msg.payloadlen = strlen(resp);

    int rc = client->publish("garage/door/response", resp_msg); // client vẫn từ MQTTCom
    printf("[MQTT DEBUG] Publish response rc = %d\n", rc);
    if (rc != 0) {
        printf("[MQTT ERROR] Publish response failed, rc=%d\n", rc);
    }
    else {
        printf("[MQTT DEBUG] Response sent: %s\n", resp);
    }

    instance->state_machine.publish_mqtt_status();
    printf("[MQTT DEBUG] Published status after command\n");
}
