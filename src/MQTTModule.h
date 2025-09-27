// MQTTModule.h
#ifndef MQTTMODULE_H
#define MQTTMODULE_H

#include <PubSubClient.h>
#include <WiFi.h>

namespace MQTTModule {
    void begin(const char* server, int port, const char* deviceId);
    void loop();
    void publish(const char* topic, const char* payload);
    void connect();
}

#endif
