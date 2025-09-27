#include "WaterLevelMonitor.h"
#include <NewPing.h>

#define MAX_DISTANCE 400 // Maximum distance in cm

namespace WaterLevelMonitor {
    // Correctly define static variables
    static int _triggerPin, _echoPin;
    static bool _sensorConnected = false;
    static NewPing _sonar(0, 0, MAX_DISTANCE); // Initialize with dummy pins

    void begin(int triggerPin, int echoPin) {
        _triggerPin = triggerPin;
        _echoPin = echoPin;
        _sonar = NewPing(triggerPin, echoPin, MAX_DISTANCE);
        
        // Check for sensor connection immediately after initializing the object
        if (isConnected()) {
            _sensorConnected = true;
            Serial.println("💧 Water Level Monitor detected.");
        } else {
            _sensorConnected = false;
            Serial.println("⚠️ Water Level Sensor not detected. Skipping module.");
        }
    }

    bool isConnected() {
        if (_triggerPin <= 0 || _echoPin <= 0) {
            return false;
        }

        // Test the sensor by performing a quick ping
        unsigned int us = _sonar.ping_cm(500); // Set a timeout of 500ms
        return (us > 0); // Returns true only if a valid echo is received
    }

    float getLevel() {
        if (!_sensorConnected) {
            return -1.0; // Indicate no reading if the sensor is not connected
        }
        
        unsigned int us = _sonar.ping_cm();
        if (us == 0) {
            return -1.0; // Indicate no reading from sensor
        }
        
        return (float)us;
    }
}