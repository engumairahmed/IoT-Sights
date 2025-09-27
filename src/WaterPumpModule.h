#ifndef WATER_PUMP_MODULE_H
#define WATER_PUMP_MODULE_H

#include <Arduino.h>

namespace WaterPumpModule {
    void begin(int motorRelayPin);
    void turnOn();
    void turnOff();
    bool isRunning();
    void update(float currentWaterLevel, float maxLevel, float minLevel, bool autoModeEnabled, bool manualOverride);
}

#endif