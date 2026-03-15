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
#include "GarageDoorController.h"
#include <iostream>

#include "LimitSwitch.h"
#include "PicoI2CDevice.h"
#include "PicoSPIBus.h"
#include "PicoSPIDevice.h"
#include "RotaryEncoder.h"
#include "StepperMotor.h"
#include "MQTTCom.h"
#include "Calibration.h"
#include "Led.h"


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

int main() {

    stdio_init_all();
    sleep_ms(1000);

    printf("\nBoot\n");

    GarageDoorController controller;

    while (true)
    {
        controller.run();
    }

    return 0;
}
