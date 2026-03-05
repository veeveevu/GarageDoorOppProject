#ifndef GARAGE_DOOR_MQTT_H
#define GARAGE_DOOR_MQTT_H

#ifdef USE_MQTT

#include <cstring>
#include "MQTTPacket.h"
#include "MQTTClient.h"
#include "IPStack.h"
#include "Countdown.h"

extern const char* topic;
extern int msg_count;
extern int mqtt_qos;

void mqtt_init();
void mqtt_loop();
void messageArrived(MQTT::MessageData& md);

#endif
#endif //GARAGE_DOOR_MQTT_H