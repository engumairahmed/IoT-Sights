#ifndef WATER_LEVEL_MONITOR_H
#define WATER_LEVEL_MONITOR_H

#include <Arduino.h>

namespace WaterLevelMonitor {
    void begin(int triggerPin, int echoPin);
    bool isConnected(); // Function to check for sensor presence
    float getLevel();
    void calibrate(float minDist, float maxDist); // Set full & empty tank distances
    float getLevelPercent();
}

#endif