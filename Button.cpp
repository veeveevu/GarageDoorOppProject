#include "Button.h"
#include <iostream>

Button::Button(uint button_pin)
    :button(button_pin)
{
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}

bool Button::is_pressed() {
    absolute_time_t now = get_absolute_time();

    bool current = !gpio_get(button);  // active low

    if (!initialized) {
        last_state = current;
        last_time = now;
        initialized = true;
        return false;
    }

    if (absolute_time_diff_us(last_time, now) < debounce_us) return false;

    bool pressed = (last_state && !current);

    if (pressed) {
        printf("[BTN DEBUG] Pin %u pressed!\n", button);
    }

    last_state = current;
    last_time = now;
    return pressed;
}