#include "LimitSwitch.h"

LimitSwitch::LimitSwitch() {
    gpio_init(Open_Switch);
    gpio_set_dir(Open_Switch, GPIO_IN);
    gpio_pull_up(Open_Switch);

    gpio_init(Close_Switch);
    gpio_set_dir(Close_Switch, GPIO_IN);
    gpio_pull_up(Close_Switch);
}

bool LimitSwitch::debounce(int switch_pin) {
    if (gpio_get(switch_pin) == 0) {
        if (switch_pin == Open_Switch) {
            printf("Open switch presses");
        } else {
            printf("Close switch presses");
        }
        while (gpio_get(switch_pin) == 0) {
            sleep_ms(DEBOUNCE_MS);
        }
        return true;
    }
    return false;
}

bool LimitSwitch::is_Open_Switch_pressed() {
    return debounce(Open_Switch);
}

bool LimitSwitch::is_Close_Switch_pressed() {
    return debounce(Close_Switch);
}
