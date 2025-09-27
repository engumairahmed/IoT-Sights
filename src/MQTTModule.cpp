
// MQTTModule.cpp
#include "MQTTModule.h"
#include <ArduinoJson.h>

namespace MQTTModule {
    static WiFiClient espClient;
    static PubSubClient client(espClient);
    static const char* _deviceId;
    static unsigned long _lastReconnectAttempt = 0;


    void begin(const char* server, int port, const char* deviceId) {
        _deviceId = deviceId;
        client.setServer(server, port);
    }

    void connect() {
        if (!client.connected()) {
            if (millis() - _lastReconnectAttempt > 5000) {
                _lastReconnectAttempt = millis();
                Serial.print("Attempting MQTT connection...");
                String clientId = String(_deviceId) + "_" + String(random(0xffff), HEX);
                if (client.connect(clientId.c_str())) {
                    Serial.println("connected");
                    String ctl = "home_iot/" + String(_deviceId) + "/control";
                    client.subscribe(ctl.c_str());
                    String stat = "home_iot/" + String(_deviceId) + "/status";
                    client.publish(stat.c_str(), "{\"status\":\"online\"}");
                } else {
                    Serial.print("failed, rc=");
                    Serial.print(client.state());
                    Serial.println(" try again in 5 seconds");
                }
            }
        }
    }

    void loop() {
        if (!client.connected()) connect();
        client.loop();
    }

    void publish(const char* topic, const char* payload) {
        if (!client.connected()) return;
        client.publish(topic, payload);
        Serial.println("Published: " + String(payload));
    }
}
