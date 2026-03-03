#include <stdio.h>
#include <string.h>
#include <cmath>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include "uart/PicoUart.h"

#include "IPStack.h"
#include "Countdown.h"
#include "MQTTClient.h"

#include <iostream>

#include "PicoI2CDevice.h"
#include "PicoSPIBus.h"
#include "PicoSPIDevice.h"

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

    printf("\nBoot\n");

#ifdef USE_MQTT
    const char* topic = "test-topic";
    //IPStack ipstack("SSID", "PASSWORD"); // example
    //IPStack ipstack("KME662", "SmartIot"); // example
    IPStack ipstack("SmartIotMQTT", "SmartIot"); // example
    auto client = MQTT::Client<IPStack, Countdown>(ipstack);

    int rc = ipstack.connect("192.168.1.10", 1883);
    if (rc != 1) {
        printf("rc from TCP connect is %d\n", rc);
    }

    printf("MQTT connecting\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"PicoW-sample";
    //data.username.cstring = (char *)"keijo";
    //data.password.cstring = (char *)"test";
    rc = client.connect(data);
    if (rc != 0) {
        printf("rc from MQTT connect is %d\n", rc);
        while (true) {
            tight_loop_contents();
        }
    }
    printf("MQTT connected\n");

    // We subscribe QoS2. Messages sent with lower QoS will be delivered using the QoS they were sent with
    rc = client.subscribe(topic, MQTT::QOS2, messageArrived);
    if (rc != 0) {
        printf("rc from MQTT subscribe is %d\n", rc);
    }
    printf("MQTT subscribed\n");

    auto mqtt_send = make_timeout_time_ms(2000);
    int mqtt_qos = 0;
    int msg_count = 0;
#endif

#ifdef USE_MQTT
    if (time_reached(mqtt_send)) {
        mqtt_send = delayed_by_ms(mqtt_send, 2000);
        if (!client.isConnected()) {
            printf("Not connected...\n");
            rc = client.connect(data);
            if (rc != 0) {
                printf("rc from MQTT connect is %d\n", rc);
            }
        }
        char buf[100];
        int rc = 0;
        MQTT::Message message;
        message.retained = false;
        message.dup = false;
        message.payload = (void*)buf;
        switch (mqtt_qos) {
        case 0:
            // Send and receive QoS 0 message
            sprintf(buf, "Msg nr: %d QoS 0 message", ++msg_count);
            printf("%s\n", buf);
            message.qos = MQTT::QOS0;
            message.payloadlen = strlen(buf) + 1;
            rc = client.publish(topic, message);
            printf("Publish rc=%d\n", rc);
            ++mqtt_qos;
            break;
        case 1:
            // Send and receive QoS 1 message
            sprintf(buf, "Msg nr: %d QoS 1 message", ++msg_count);
            printf("%s\n", buf);
            message.qos = MQTT::QOS1;
            message.payloadlen = strlen(buf) + 1;
            rc = client.publish(topic, message);
            printf("Publish rc=%d\n", rc);
            ++mqtt_qos;
            break;
#if MQTTCLIENT_QOS2
case 2:
    // Send and receive QoS 2 message
    sprintf(buf, "Msg nr: %d QoS 2 message", ++msg_count);
    printf("%s\n", buf);
    message.qos = MQTT::QOS2;
    message.payloadlen = strlen(buf) + 1;
    rc = client.publish(topic, message);
    printf("Publish rc=%d\n", rc);
    ++mqtt_qos;
    break;
#endif
default:
    mqtt_qos = 0;
    break;
            }
        }

    cyw43_arch_poll(); // obsolete? - see below
    client.yield(100); // socket that client uses calls cyw43_arch_poll()
#endif
    /*
    while (true) {
        tight_loop_contents();
    }
    */
    std::cout << "Hello";
}
