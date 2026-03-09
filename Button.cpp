#include "Button.h"
#include <cstdio>

Button::Button(uint button_pin)
    :button(button_pin)
{
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}

bool Button::is_pressed() {

    bool current_state = !gpio_get(button);  // active low

    absolute_time_t now = get_absolute_time();

    //check if its been longer than debounce_us
    if (absolute_time_diff_us(last_time, now) > debounce_us) {

        //out of waiting period, the state has changed
        if (current_state != last_state) {
            last_state = current_state;
            last_time = now;

            if (current_state == true) {
                //printf("[BUTTON] Button pin %d is pressed.\n", button);
                return true;
            }
        }

    //if NO then in cooling down period -> if it stills bouncing reset the timer
    } else {
        if (current_state != last_state) {
            last_time = now;
            last_state = current_state;
        }
    }
    return false;
}