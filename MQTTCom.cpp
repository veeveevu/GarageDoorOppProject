//
// Created by vuhav on 04/03/2026.
//

#include "MQTTCom.h"
#include "pico/time.h"
#include <cstring>

#ifdef USE_MQTT
    const char* topic = "test-topic";

    static IPStack* ipstack = nullptr;
    static MQTT::Client<IPStack, Countdown>* client = nullptr;

    int mqtt_qos = 0;
    int msg_count = 0;

    static absolute_time_t mqtt_send;
    static MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

    void mqtt_init()
    {
        //IPStack ipstack("SSID", "PASSWORD"); // example
        //IPStack ipstack("KME662", "SmartIot"); // example
        //IPStack ipstack("SmartIotMQTT", "SmartIot"); // example
        //IPStack ipstack("MP-IOT", "3QDaDHLn10"); // Karamalmi
        //IPSTack ipstack("MP-IOT", "ID2vOcYrWi"); //Myyrmaki
        ipstack = new IPStack("TP-Link_FFDC", "61172937");
        client  = new MQTT::Client<IPStack, Countdown>(*ipstack);

        //int rc = ipstack.connect("192.168.1.10", 1883);
        int rc = ipstack->connect("192.168.1.104", 1883); //Tram's Home IP
        if (rc != 1) {
            printf("rc from TCP connect is %d\n", rc);
        }

        printf("MQTT connecting\n");

        data.MQTTVersion = 3;
        data.clientID.cstring = (char*)"PicoW-sample";
        //data.username.cstring = (char *)"keijo";
        //data.password.cstring = (char *)"test";
        rc = client->connect(data);
        if (rc != 0) {
            printf("rc from MQTT connect is %d\n", rc);
            while (true) {
                tight_loop_contents();
            }
        }
        printf("MQTT connected\n");

        // We subscribe QoS2. Messages sent with lower QoS will be delivered using the QoS they were sent with
        rc = client->subscribe(topic, MQTT::QOS2, messageArrived);
        if (rc != 0) {
            printf("rc from MQTT subscribe is %d\n", rc);
        }
        printf("MQTT subscribed\n");
        mqtt_send = make_timeout_time_ms(2000);
    }

    void mqtt_loop()
    {
        if (time_reached(mqtt_send)) {
            mqtt_send = delayed_by_ms(mqtt_send, 2000);
            if (!client->isConnected()) {
                printf("Not connected...\n");
                int rc = client->connect(data);
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
                rc = client->publish(topic, message);
                printf("Publish rc=%d\n", rc);
                ++mqtt_qos;
                break;
            case 1:
                // Send and receive QoS 1 message
                sprintf(buf, "Msg nr: %d QoS 1 message", ++msg_count);
                printf("%s\n", buf);
                message.qos = MQTT::QOS1;
                message.payloadlen = strlen(buf) + 1;
                rc = client->publish(topic, message);
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
                rc = client->publish(topic, message);
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
        client->yield(100); // socket that client uses calls cyw43_arch_poll()
    }
#endif