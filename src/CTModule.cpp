
// CTModule.cpp
#include "CTModule.h"

// Static members initialization
int CTModule::_ctPin = -1;
float CTModule::_calibration = 185.0f; // This value will be updated with calibration
float CTModule::_rmsCurrent = 0.0f;
bool CTModule::_isConnected = false;
int CTModule::_noLoadOffset = 0; // New member to store the DC offset

// --- Helper function to measure the no-load ADC offset ---
int CTModule::setupNoLoadOffset() {
    long sum = 0;
    int sampleCount = 0;
    // Average a large number of samples to get a stable offset
    for (int i = 0; i < 1000; i++) {
        sum += analogRead(_ctPin);
        sampleCount++;
        delayMicroseconds(50);
    }
    return sum / sampleCount;
}

// Initialize the CT module on a specified pin with a calibration factor
void CTModule::begin(int ctPin, float calibration) {
    _ctPin = ctPin;
    _calibration = calibration;
    pinMode(_ctPin, INPUT);

    // Initial check for a valid reading and store the no-load offset
    _isConnected = (analogRead(_ctPin) > 0);
    if (_isConnected) {
        _noLoadOffset = setupNoLoadOffset();
        Serial.printf("Sensor calibrated with no-load offset: %d\n", _noLoadOffset);
    }
}

// Private helper function to get the raw RMS value from the ADC
float CTModule::getRawRMS() {
    long sum = 0;
    int sampleCount = 0;
    
    // Sample for 100ms
    unsigned long startTime = millis();
    while (millis() - startTime < 100) {
        int adcValue = analogRead(_ctPin);
        // Use the measured offset instead of a fixed 2047
        int offsetValue = adcValue - _noLoadOffset;
        sum += (long)offsetValue * offsetValue;
        sampleCount++;
    }

    // Safety check to prevent division by zero and 'nan' values
    if (sampleCount == 0) {
        return 0.0f;
    }
    
    float rms_sq = (float)sum / sampleCount;
    return sqrt(rms_sq);
}

// Updates sensor readings and performs RMS calculations
void CTModule::update() {
    if (!_isConnected) return;
    
    float rms_raw = getRawRMS();
    _rmsCurrent = rms_raw / _calibration; 
}

// Returns the measured RMS current in Amperes
float CTModule::getCurrent() {
    // Add a small threshold to filter out electrical noise
    static const float NO_LOAD_THRESHOLD = 0.50f; // Increase this value to filter out the noise
    if (_rmsCurrent < NO_LOAD_THRESHOLD) {
        return 0.0f;
    }
    return _rmsCurrent;
}

// Checks if the CT module is connected and working
bool CTModule::isConnected() {
    return _isConnected;
}

// --- Dynamic Calibration Function ---
void CTModule::calibrate(float knownCurrent) {
    // This function should be called with a known load connected.
    
    float rms_raw = 0.0f;
    int measurementCount = 0;
    
    // Average 10 readings to get a stable value
    for(int i = 0; i < 10; i++) {
        rms_raw += getRawRMS();
        measurementCount++;
        delay(100); // Wait between readings
    }
    
    if(measurementCount > 0) {
        rms_raw /= measurementCount;
        
        // Calculate the new calibration factor: New Calibration = Raw RMS Value / Known Current
        _calibration = rms_raw / knownCurrent;
        
        Serial.printf("✅ New CT calibration factor: %.2f\n", _calibration);
    }
}