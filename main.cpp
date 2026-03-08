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

int main() {

    // Initialize chosen serial port
    stdio_init_all();
    sleep_ms(1000);

    printf("\nBoot\n");

    Button sw0{SW0_PIN};
    while (true) {
        sw0.is_pressed();
    }

    //GarageDoorController controller;
    //controller.run();

    //Led led1(20);

    //StepperMotor m(2, 3, 6, 13);

    //LimitSwitch lm(14, 15);

    //RotaryEncoder re(27, 28);

    //Calibration clb(m, lm, re);


    //m.run_steps(1000, MotorDirection::ToOpen);
    //clb.do_calibration();

/*
    while (true) {
        //tight_loop_contents();
        mqtt_loop();
    }

*/
    return 0;
}
