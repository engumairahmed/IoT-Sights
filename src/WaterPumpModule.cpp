#include "WaterPumpModule.h"

namespace WaterPumpModule {
    static int _motorRelayPin;
    static bool _isRunning = false;

    void begin(int motorRelayPin) {
        _motorRelayPin = motorRelayPin;
        pinMode(_motorRelayPin, OUTPUT);
        digitalWrite(_motorRelayPin, LOW); // Assuming LOW is OFF
        Serial.println("💧 Water Pump Module Ready");
    }

    void turnOn() {
        if (!_isRunning) {
            digitalWrite(_motorRelayPin, HIGH); // Assuming HIGH is ON
            _isRunning = true;
            Serial.println("Motor ON");
        }
    }

    void turnOff() {
        if (_isRunning) {
            digitalWrite(_motorRelayPin, LOW); // Assuming LOW is OFF
            _isRunning = false;
            Serial.println("Motor OFF");
        }
    }

    bool isRunning() {
        return _isRunning;
    }
    
    // Main logic function to be called in the main loop
    void update(float currentWaterLevel, float maxLevel, float minLevel, bool autoModeEnabled, bool manualOverride) {
        // Safety check: Never turn on if tank is full
        if (currentWaterLevel >= maxLevel) {
            if (_isRunning) {
                turnOff();
                Serial.println("✅ Safety: Tank full, motor stopped.");
            }
            return;
        }

        // Handle manual override
        if (manualOverride) {
            if (currentWaterLevel < maxLevel) { // Only allow manual ON if not full
                turnOn();
            } else {
                Serial.println("⚠️ Manual override failed: Tank is already full.");
            }
            return; // Manual override takes precedence over auto mode
        }

        // Handle automatic mode
        if (autoModeEnabled) {
            if (currentWaterLevel <= minLevel && !_isRunning) {
                turnOn();
            } else if (currentWaterLevel >= maxLevel && _isRunning) {
                turnOff();
            }
        }
    }
}