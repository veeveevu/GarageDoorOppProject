//
// Created by vuhav on 04/03/2026.
//

#include "MQTTCom.h"
#include "pico/time.h"
#include <cstring>
#include "GarageDoorController.h"

#ifdef USE_MQTT
    const char* topic = "garage/door/command";

    static IPStack* ipstack = nullptr;
    MQTT::Client<IPStack, Countdown>* client = nullptr;

    int mqtt_qos = 0;
    int msg_count = 0;

    static absolute_time_t mqtt_send;
    static MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    std::string hostname = "192.168.1.104";

    void mqtt_init()
    {
        //IPStack ipstack("SSID", "PASSWORD"); // example
        //IPStack ipstack("KME662", "SmartIot"); // example
        //IPStack ipstack("SmartIotMQTT", "SmartIot"); // example
        //IPStack ipstack("MP-IOT", "3QDaDHLn10"); // Karamalmi
        //IPSTack ipstack("MP-IOT", "ID2vOcYrWi"); //Myyrmaki
        //ipstack = new IPStack("MP-IOT", "ID2vOcYrWi"); // Karamalmi
        ipstack = new IPStack("TP-Link_FFDC", "61172937");
        client  = new MQTT::Client<IPStack, Countdown>(*ipstack);

        //int rc = ipstack.connect("192.168.1.10", 1883);
        //int rc = ipstack->connect("10.161.4.56", 1883); //Tram's School IP
        int rc = ipstack->connect(hostname.c_str(), 1883); //Tram's Home IP
        printf("[MQTT DEBUG] TCP connect rc = %d\n", rc);
        if (rc != 0) {
            printf("[MQTT ERROR] rc from TCP connect is %d\n", rc);
            return;
        }

        printf("[MQTT DEBUG] Waiting for TCP connection...\n");
        bool tcp_ok = false;
        auto timeout = make_timeout_time_ms(10000);
        while (!time_reached(timeout)) {
            cyw43_arch_poll();
            sleep_ms(10);
            int test = client->connect(data);
            if (test == 0) {
                tcp_ok = true;
                printf("[MQTT DEBUG] MQTT connected!\n");
                break;
            }
        }

        if (!tcp_ok) {
            printf("[MQTT ERROR] Connection timeout!\n");
            return;
        }
        //printf("[MQTT DEBUG] TCP wait done, trying MQTT connect\n");

        //printf("[MQTT DEBUG] MQTT connecting\n");

        data.MQTTVersion = 3;
        data.clientID.cstring = (char*)"PicoW-sample";
        //data.username.cstring = (char *)"keijo";
        //data.password.cstring = (char *)"test";
        /*
        rc = client->connect(data);
        printf("[MQTT DEBUG] MQTT connect rc = %d\n", rc);
        if (rc != 0) {
            printf("[MQTT ERROR] rc from MQTT connect is %d\n", rc);
            return;
        }
        */
        printf("[MQTT DEBUG] MQTT connected\n");

        // We subscribe QoS2. Messages sent with lower QoS will be delivered using the QoS they were sent with
        rc = client->subscribe(topic, MQTT::QOS1, GarageDoorController::messageArrived);
        if (rc != 0) {
            printf("[MQTT ERROR] rc from MQTT subscribe is %d\n", rc);
        }
        printf("[MQTT DEBUG] MQTT subscribed to: %s\n", topic);
        mqtt_send = make_timeout_time_ms(2000);
    }

    void mqtt_loop()
    {
        if (time_reached(mqtt_send)) {
            mqtt_send = delayed_by_ms(mqtt_send, 2000);
            if (!client->isConnected()) {
                printf("[MQTT DEBUG] Not connected...\n");

                ipstack->disconnect();
                int rc = ipstack->connect(hostname.c_str(), 1883);
                if (rc != ERR_OK) {
                    printf("[MQTT ERROR] TCP reconnect failed: %d\n", rc);
                    return;
                }

                auto timeout = make_timeout_time_ms(2000);
                while (!time_reached(timeout)) {
                    cyw43_arch_poll();
                    sleep_ms(10);
                }

                rc = client->connect(data);
                if (rc != 0) {
                    printf("[MQTT ERROR] rc from MQTT connect is %d\n", rc);
                    return;
                }
            }
            char buf[100];
            int rc = 0;
            MQTT::Message message;
            message.retained = false;
            message.dup = false;
            message.payload = (void*)buf;
            /*
            switch (mqtt_qos) {
            case 0:
                // Send and receive QoS 0 message
                sprintf(buf, "Msg nr: %d QoS 0 message", ++msg_count);
                //printf("[MQTT DEBUG] Sending QoS0: %s\n", buf);
                message.qos = MQTT::QOS0;
                message.payloadlen = strlen(buf) + 1;
                rc = client->publish(topic, message);
                //printf("[MQTT DEBUG] Publish rc=%d\n", rc);
                ++mqtt_qos;
                break;
            case 1:
                // Send and receive QoS 1 message
                sprintf(buf, "Msg nr: %d QoS 1 message", ++msg_count);
                //printf("[MQTT DEBUG] Sending QoS1: %s\n", buf);
                message.qos = MQTT::QOS1;
                message.payloadlen = strlen(buf) + 1;
                rc = client->publish(topic, message);
                //printf("[MQTT DEBUG] Publish rc=%d\n", rc);
                ++mqtt_qos;
                break;

        #if MQTTCLIENT_QOS2
            case 2:
                // Send and receive QoS 2 message
                sprintf(buf, "Msg nr: %d QoS 2 message", ++msg_count);
                //printf("[MQTT DEBUG] Sending QoS2: %s\n", buf);
                message.qos = MQTT::QOS2;
                message.payloadlen = strlen(buf) + 1;
                rc = client->publish(topic, message);
                //printf("[MQTT DEBUG] Publish rc=%d\n", rc);
                ++mqtt_qos;
                break;
        #endif
            default:
                mqtt_qos = 0;
                break;
            }
            */
        }
        cyw43_arch_poll(); // obsolete? - see below
        client->yield(1); // socket that client uses calls cyw43_arch_poll()
        //printf("[MQTT DEBUG] Yield completed\n");
    }

    bool is_mqtt_connected() {
        return client && client->isConnected();
    }
#endif