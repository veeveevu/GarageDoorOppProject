#include "Button.h"

Button::Button(uint button_pin)
    :button(button_pin)
{
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}