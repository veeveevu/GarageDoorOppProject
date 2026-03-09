#include "GarageDoorController.h"
#include <cstdio>
#include <string>
/*
**The full journey of one button press, step by step:**

1. User presses SW1 button physically

2. GarageController::checkButtons() runs
   → gpio reads LOW on SW1 pin
   → "oh something happened, I'll label it ButtonPressed"
   → calls m_stateMachine.handleEvent(Event::ButtonPressed)

3. DoorStateMachine::handleEvent() receives it
   → checks current state: Closed
   → checks event: ButtonPressed
   → finds the matching row in the table
   → changes m_state to Opening

4. GarageController::reactToState() runs
   → reads state: Opening
   → calls m_motor.step(Direction::Clockwise)

   */
GarageDoorController* GarageDoorController::instance = nullptr;

GarageDoorController::GarageDoorController()
    : calibration(motor, limits, encoder) {
    printf("[CONSTR] Start constructor...\n");

/* have done these in smaller classes!
    // Init buttons pull-up
    printf("[CONSTR] Init buttons...\n");
    gpio_init(SW0_PIN);
    gpio_set_dir(SW0_PIN, GPIO_IN);
    gpio_pull_up(SW0_PIN);
    gpio_init(SW1_PIN);
    gpio_set_dir(SW1_PIN, GPIO_IN);
    gpio_pull_up(SW1_PIN);
    gpio_init(SW2_PIN);
    gpio_set_dir(SW2_PIN, GPIO_IN);
    gpio_pull_up(SW2_PIN);

    // Init LEDs
    printf("[CONSTR] Init LEDs...\n");
    gpio_init(LED_OPEN_PIN);
    gpio_set_dir(LED_OPEN_PIN, GPIO_OUT);
    gpio_init(LED_CLOSE_PIN);
    gpio_set_dir(LED_CLOSE_PIN, GPIO_OUT);
    gpio_init(LED_ERROR_PIN);
    gpio_set_dir(LED_ERROR_PIN, GPIO_OUT);
*/

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
    check_stuck();
    react_to_state();
    update_leds();
    mqtt_loop();
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

    if (sw0.is_pressed() && sw2.is_pressed()) {
        printf("[BTN] SW0 + SW2 pressed → Calibration!\n");
        state_machine.handle_event(Event::SW0_SW2_PRESSED);
    }
    if (sw1.is_pressed()) {
        printf("[BTN] SW1 pressed!\n");
        state_machine.handle_event(Event::SW1_PRESSED);
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
        if (absolute_time_diff_us(last_encoder_change, get_absolute_time()) > stuck_timeout_us) {
            state_machine.handle_event(Event::STUCK_FOUND);
            printf("[STUCK] No movement detected!\n");
        }
    }
}

void GarageDoorController::react_to_state() {
    auto st = state_machine.get_current_state();

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
    printf("[CALIB] Starting calibration...\n");
    calibration.do_calibration();

    total_motor_steps = calibration.get_motor_counter();
    total_encoder_turns = calibration.get_encoder_counter();
    current_pos = 0;

    printf("[CALIB] Total motor steps: %d\n", total_motor_steps);
    printf("[CALIB] Total encode turns: %d\n", total_encoder_turns);

    state_machine.handle_event(Event::CALIBRATION_COMPLETE_SUCCESS);
    save_state();
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
