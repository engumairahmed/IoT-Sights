#ifndef WATER_LEVEL_MONITOR_H
#define WATER_LEVEL_MONITOR_H

#include <Arduino.h>

namespace WaterLevelMonitor {
    void begin(int triggerPin, int echoPin);
    float getLevel();
    bool isConnected(); // New function to check for sensor presence
}

#endif