#ifndef GARAGE_DOOR_MQTT_H
#define GARAGE_DOOR_MQTT_H


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

#endif //GARAGE_DOOR_MQTT_H