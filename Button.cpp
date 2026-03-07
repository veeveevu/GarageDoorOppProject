#include "Button.h"

Button::Button(uint button_pin)
    :button(button_pin)
{
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}

bool Button::is_pressed() {
    static absolute_time_t last_time = nil_time;
    static bool last_state = true;

    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_time, now) < 30000) return false;  // 30ms debounce

    bool current = !gpio_get(button);  // active low (pull-up)
    bool pressed = (last_state && !current);  // falling edge

    last_state = current;
    last_time = now;

    return pressed;
}