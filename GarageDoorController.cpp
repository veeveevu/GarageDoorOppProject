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
        react_to_state();
        check_stuck();
        check_finish_moving();
    }
    update_leds();
    //mqtt_loop();
    sleep_us(1000);
}

void GarageDoorController::compute_ratio() {
    if (total_motor_steps > 0) {
        step_ratio = (float)total_encoder_turns / (float)total_motor_steps;
        printf("[RATIO] Encoder ticks per motor step: %.4f\n", step_ratio);
    } else {
        step_ratio = 0.0f;
        printf("[RATIO] Warning: total_motor_steps is 0, ratio not set!\n");
    }
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
    //bool sw2_pressed = sw2.is_pressed();

    if (sw0_pressed) {
            printf("[BTN] SW0 + SW2 pressed → Calibration!\n");
            state_machine.handle_event(Event::SW0_SW2_PRESSED);
            return;
    }
    if (sw1_pressed) {
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
    int events_this_cycle = 0;
    while (encoder.getEvent(dir)) {  // Keep reading until queue is empty
        current_pos += dir;
        encoder_ticks_since_check++;
        last_encoder_change = get_absolute_time();
        events_this_cycle++;
    }

    // Debug output
    if (events_this_cycle > 0) {
        static int total_events = 0;
        total_events += events_this_cycle;
        if (total_events % 100 == 0) {
            printf("[ENC] Got %d events this cycle, total: %d, pos: %d\n",
                   events_this_cycle, total_events, current_pos);
        }
    }
}

void GarageDoorController::check_stuck() {
    auto st = state_machine.get_current_state();
    if (st != DoorState::OPENING && st != DoorState::CLOSING) {
        return;
    }
    if (step_ratio <= 0.0f || total_motor_steps <= 0) {
        printf("[STUCK] Ratio not initialized, skipping check\n");
        return;
    }

    if (motor_steps_since_check < 50) {
        return;
    }

    // Every RATIO_CHECK_INTERVAL motor steps, evaluate the ratio
    if (motor_steps_since_check >= RATIO_CHECK_INTERVAL) {
        float expected_ticks = RATIO_CHECK_INTERVAL * step_ratio;
        float actual_ticks   = (float)encoder_ticks_since_check;

        printf("[RATIO] Expected: %.1f ticks, Got: %.1f ticks\n",
               expected_ticks, actual_ticks);

        if (actual_ticks < expected_ticks * SLIP_THRESHOLD && expected_ticks > 1.0f) {
            // Got less than 50% of expected encoder ticks → door is slipping or jammed
            printf("[STUCK] Ratio-based: expected %.1f ticks but got %.1f!\n",
                   expected_ticks, actual_ticks);
            state_machine.handle_event(Event::STUCK_FOUND);
            return;
        }

        // Reset for next interval regardless
        motor_steps_since_check   = 0;
        encoder_ticks_since_check = 0;
    }
}

void GarageDoorController::react_to_state() {
    auto st = state_machine.get_current_state();
    static DoorState last_state = DoorState::NOT_CALIBRATED;

    // Reset encoder timer when entering movement states
    if ((st == DoorState::OPENING || st == DoorState::CLOSING) &&
        (last_state != st)) {
        last_encoder_change = get_absolute_time();
        motor_steps_since_check   = 0;
        encoder_ticks_since_check = 0;
        printf("[REACT] Started movement, reset stuck timer\n");
        printf("[REACT] Ratio: %.4f, Total motor steps: %d, Total encoder: %d\n",
               step_ratio, total_motor_steps, total_encoder_turns);
        printf("[REACT] Started movement, reset stuck timer\n");
        }
    last_state = st;

    switch (st) {
    case DoorState::CALIBRATING:
        perform_calibration();
        break;

    case DoorState::OPENING:
        motor.step(Direction::ToOpen);
        current_pos++;
        motor_steps_since_check++;
        if (motor_steps_since_check % 500 == 0) {
            printf("[MOTOR] Opening: pos=%d, motor_steps=%d, encoder_ticks=%d\n",
                   current_pos, motor_steps_since_check, encoder_ticks_since_check);
        }
        break;

    case DoorState::CLOSING:
        motor.step(Direction::ToClose);
        current_pos--; //???? idk understand it yet
        motor_steps_since_check++;
        if (motor_steps_since_check % 500 == 0) {
            printf("[MOTOR] Closing: pos=%d, motor_steps=%d, encoder_ticks=%d\n",
                   current_pos, motor_steps_since_check, encoder_ticks_since_check);
        }
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

void GarageDoorController::check_finish_moving() {
    auto st = state_machine.get_current_state();

    // Only relevant when door is actually moving
    if (st != DoorState::OPENING && st != DoorState::CLOSING) {
        return;
    }

    if (st == DoorState::OPENING) {
        // current_pos counts UP from 0 (closed) toward total_motor_steps (open)
        // Stop 300 steps before the physical limit switch body
        if (current_pos >= (total_motor_steps - 300)) {
            printf("[POS] Opening complete at pos=%d (limit=%d)\n",
                   current_pos, total_motor_steps - 300);
            state_machine.handle_event(Event::FINISH_MOVING);
            // Reset counters for next movement
            motor_steps_since_check   = 0;
            encoder_ticks_since_check = 0;
        }
    }

    if (st == DoorState::CLOSING) {
        // current_pos counts DOWN from total_motor_steps toward 0 (closed)
        // Stop 300 steps before the physical limit switch body
        if (current_pos <= 300) {
            printf("[POS] Closing complete at pos=%d\n", current_pos);
            state_machine.handle_event(Event::FINISH_MOVING);
            // Reset counters for next movement
            motor_steps_since_check   = 0;
            encoder_ticks_since_check = 0;
        }
    }
}

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

    compute_ratio();
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
    uint8_t buf[8] = {0};
    if (eeprom_read_multi(EEPROM_STEPS_ADDR, buf, 8)) {
        int loaded_steps = (buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
        int loaded_turns = (buf[4]<<24)|(buf[5]<<16)|(buf[6]<<8)|buf[7];

        // Only use loaded values if they look reasonable
        if (loaded_steps > 0 && loaded_steps < 14000) {
            total_motor_steps = loaded_steps;
            total_encoder_turns = loaded_turns;
            compute_ratio();
            printf("[EEPROM] Loaded motor steps: %d, encoder turns: %d\n", total_motor_steps, total_encoder_turns);
        } else {
            printf("[EEPROM] Motor steps invalid, will need recalibration\n");
        }
    }
}

void GarageDoorController::save_state() {
    state_machine.save_state_to_eeprom();
    uint8_t buf[8];
    buf[0] = (total_motor_steps >> 24) & 0xFF;
    buf[1] = (total_motor_steps >> 16) & 0xFF;
    buf[2] = (total_motor_steps >> 8)  & 0xFF;
    buf[3] = (total_motor_steps)       & 0xFF;
    buf[4] = (total_encoder_turns >> 24) & 0xFF;
    buf[5] = (total_encoder_turns >> 16) & 0xFF;
    buf[6] = (total_encoder_turns >> 8)  & 0xFF;
    buf[7] = (total_encoder_turns)       & 0xFF;
    eeprom_write_multi(EEPROM_STEPS_ADDR, buf, 8);
    printf("[EEPROM] Saved motor steps: %d, encoder turns: %d\n", total_motor_steps, total_encoder_turns);
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

