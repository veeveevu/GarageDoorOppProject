#include "LimitSwitch.h"

LimitSwitch::LimitSwitch(uint open_switch_pin, uint close_switch_pin)
    : Open_Switch(open_switch_pin), Close_Switch(close_switch_pin) {
    gpio_init(Open_Switch);
    gpio_set_dir(Open_Switch, GPIO_IN);
    gpio_pull_up(Open_Switch);

    gpio_init(Close_Switch);
    gpio_set_dir(Close_Switch, GPIO_IN);
    gpio_pull_up(Close_Switch);
}

bool LimitSwitch::debounce(uint switch_pin, const char* switch_name) {
    if (gpio_get(switch_pin) == 0) {
        //sleep_ms(DEBOUNCE_MS);

        if (gpio_get(switch_pin) == 0) {
            printf("%s switch pressed\n", switch_name);
            return true;
        }
    }
    return false;
}

bool LimitSwitch::is_Open_Switch_pressed() {
    bool pressed = (gpio_get(Open_Switch) == 0);
    if (pressed && !open_was_pressed) {
        open_was_pressed = true;
        printf("Open switch pressed\n");
        return true;
    }
    if (!pressed) {
        open_was_pressed = false;
    }
    return false;
}

bool LimitSwitch::is_Close_Switch_pressed() {
    bool pressed = (gpio_get(Close_Switch) == 0);
    if (pressed && !close_was_pressed) {
        close_was_pressed = true;
        printf("Close switch pressed\n");
        return true;
    }
    if (!pressed) {
        close_was_pressed = false;
    }
    return false;
}
