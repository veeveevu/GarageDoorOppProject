#include "GarageDoorController.h"
#include <cstdio>
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

GarageDoorController::GarageDoorController() {
    // Init buttons pull-up
    gpio_init(SW0_PIN); gpio_set_dir(SW0_PIN, GPIO_IN); gpio_pull_up(SW0_PIN);
    gpio_init(SW1_PIN); gpio_set_dir(SW1_PIN, GPIO_IN); gpio_pull_up(SW1_PIN);
    gpio_init(SW2_PIN); gpio_set_dir(SW2_PIN, GPIO_IN); gpio_pull_up(SW2_PIN);

    // Init LEDs
    gpio_init(LED_OPEN_PIN); gpio_set_dir(LED_OPEN_PIN, GPIO_OUT);
    gpio_init(LED_CLOSE_PIN); gpio_set_dir(LED_CLOSE_PIN, GPIO_OUT);
    gpio_init(LED_ERROR_PIN); gpio_set_dir(LED_ERROR_PIN, GPIO_OUT);

    mqtt_init();
    eeprom_log_init();

    load_state();
}

void GarageDoorController::run() {
    while (true) {
        check_buttons();
        check_limits_and_encoder();
        check_stuck();
        react_to_state();
        update_leds();
        mqtt_loop();
        sleep_us(5000);
    }
}

void GarageDoorController::check_buttons() {
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_debounce, now) < debounce_us) return;

    if (sw0.is_pressed() && sw2.is_pressed()) {
        state_machine.handle_event(Event::SW0_SW2_PRESSED);
    }
    if (sw1.is_pressed()) {
        state_machine.handle_event(Event::SW1_PRESSED);
    }

    last_debounce = now;
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

void GarageDoorController::update_leds() {
    auto st = state_machine.get_current_state();

    if (st == DoorState::DOOR_OPENED) {
        led_open.turn_led_on();
        led_close.turn_led_off();
    } else if (st == DoorState::DOOR_CLOSED) {
        led_close.turn_led_on();
        led_open.turn_led_off();
    } else {
        led_open.turn_led_off();
        led_close.turn_led_off();
    }

    if (st == DoorState::ERROR) {
        led_error.blink_led();
    } else {
        led_error.turn_led_off();
    }
}

void GarageDoorController::load_state() {
    state_machine.load_state_from_eeprom();
}

void GarageDoorController::save_state() {
    state_machine.save_state_to_eeprom();
}