#include <cstdio>
#include <cstring>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include "uart/PicoUart.h"

#include "IPStack.h"
#include "Countdown.h"
#include "MQTTClient.h"

#include <iostream>

#include "LimitSwitch.h"
#include "PicoI2CDevice.h"
#include "PicoSPIBus.h"
#include "PicoSPIDevice.h"
#include "RotaryEncoder.h"
#include "StepperMotor.h"
#include "Calibration.h"

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#if 0
#define UART_NR 0
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#else
#define UART_NR 1
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#endif

#define BAUD_RATE 9600
#define STOP_BITS 1 // for simulator
//#define STOP_BITS 2 // for real system


void messageArrived(MQTT::MessageData& md) {
    MQTT::Message& message = md.message;

    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n",
           message.qos, message.retained, message.dup, message.id);
    printf("Payload %s\n", (char*)message.payload);
}


int main() {
    const uint led_pin = 22;
    const uint button = 9;

    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);

    // Initialize chosen serial port
    stdio_init_all();
    sleep_ms(2000);

    printf("\nBoot\n");

    /*
    while (true) {
        tight_loop_contents();
    }
    */
    StepperMotor m(2, 3, 6, 13);

    LimitSwitch lm(14, 15);

    RotaryEncoder re(27, 28);

    Calibration clb(m, lm, re);


    clb.do_calibration();

}
