#ifndef WIFIMODULE_H
#define WIFIMODULE_H

#include <Arduino.h>

namespace WiFiModule {
    void begin(int redLedPin, int greenLedPin);
    bool isConnected();
    String getIP();
    void resetSettings();
    String getSSID();
    String getPassword();
    String getUniqueId();
}

#endif