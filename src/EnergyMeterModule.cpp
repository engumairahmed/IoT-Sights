
// EnergyMeterModule.cpp

#include "EnergyMeterModule.h"
#include <Arduino.h>
#include "ACS712.h"

namespace EnergyMeterModule {
    // ACS712 object
    static ACS712 acs(0, 0, 0, 0); // Placeholder, will be configured in begin()
    
    // Data storage for calculations
    static float _totalEnergykWh = 0.0;
    static float _peakPower = 0.0;
    static float _lastSampledPower = 0.0;
    static unsigned long _lastUpdateTime = 0;
    static int _acs712Pin;
    static bool _sensorConnected = false;
    static float _voltageCalibration = 220.0;
    static float _noLoadOffset = 0.0; // New variable to store the zero-offset

    bool isConnected() {
        if (_acs712Pin <= 0) {
            return false;
        }
        
        // Take a stable reading to determine if the sensor is present.
        int sensorReading = analogRead(_acs712Pin);
        
        // Check if the reading is not at the ADC's max or min
        if (sensorReading < 100 || sensorReading > 4000) {
            return false;
        }
        
        return true; 
    }

    void begin(int acs712Pin, float voltageCalibration, float sensitivity) {
        _acs712Pin = acs712Pin;
        _voltageCalibration = voltageCalibration;

        if (isConnected()) {
            _sensorConnected = true;

            // Initialize the ACS712 object with ESP32-specific values
            acs = ACS712(acs712Pin, 3.3, 4095, sensitivity);
            
            // Perform manual offset calibration with no load
            _noLoadOffset = acs.mA_AC();
            Serial.printf("Sensor calibrated with no-load offset: %.2f mA\n", _noLoadOffset);

            Serial.println("⚡ Energy Meter detected.");
        } else {
            _sensorConnected = false;
            Serial.println("⚠️ Energy Meter not detected. Skipping module.");
        }
    }

    
    // Measure RMS current over N samples
    float readRMSCurrent(int numSamples = 2000) {
        if (!_sensorConnected) return 0.0;

        double sumSquares = 0.0;
        for (int i = 0; i < numSamples; i++) {
            int sample = analogRead(_acs712Pin);
            float current_mA = acs.mA_AC(sample) - _noLoadOffset;
            sumSquares += current_mA * current_mA;
            delayMicroseconds(50); // Short delay for sampling (adjust if needed)
        }

        double meanSquare = sumSquares / numSamples;
        double rms_mA = sqrt(meanSquare);
        return rms_mA / 1000.0; // Convert mA → A
    }

    void update() {
        if (!_sensorConnected) return; // Exit if the sensor is not connected
        
        if (millis() - _lastUpdateTime >= 1000) { // Update every second
            _lastUpdateTime = millis();
            
            // Get the AC current from the ACS712 sensor in milliamps
            int current_mA = acs.mA_AC();
            
            // Apply the zero-load offset correction
            float current_corrected_mA = current_mA - _noLoadOffset;
            float current_A = (float)current_corrected_mA / 1000.0;

            // Calculate apparent power
            float power = current_A * _voltageCalibration;
            
            // Ensure power doesn't show negative values due to noise
            if (power < 0) power = 0;
            if (power > _peakPower) _peakPower = power;
            
            // Calculate energy consumed in the last second (Wh)
            float energy_wh = power * (1.0 / 3600.0);
            _totalEnergykWh += energy_wh / 1000.0;

            _lastSampledPower = power;
        }
    }

    // void update() {
    //     if (!_sensorConnected) return;

    //     if (millis() - _lastUpdateTime >= 1000) { // Update every second
    //         _lastUpdateTime = millis();

    //         float current_A = readRMSCurrent(2000);
    //         if (current_A < 0.05) current_A = 0.0;

    //         // Calculate apparent power
    //         float power = current_A * _voltageCalibration;

    //         // Ensure power doesn't show negative values due to noise
    //         if (power < 0) power = 0;
    //         if (power > _peakPower) _peakPower = power;

    //         // Energy in Wh
    //         float energy_wh = power * (1.0 / 3600.0);
    //         _totalEnergykWh += energy_wh / 1000.0;

    //         _lastSampledPower = power;
    //     }
    // }
    
    float getCurrent() {
        // Return corrected current in Amps
        int current_mA = acs.mA_AC();
        float current_corrected_mA = current_mA - _noLoadOffset;
        return (float)current_corrected_mA / 1000.0;

        // return readRMSCurrent();    
    }
    
    float getPower() {
        return _lastSampledPower;
    }
    
    float getCumulativeEnergy() {
        return _totalEnergykWh;
    }

    float getPeakPower() {
        return _peakPower;
    }
}