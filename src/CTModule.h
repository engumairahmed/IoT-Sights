
// CTModule.h

#ifndef CT_MODULE_H
#define CT_MODULE_H

#include <Arduino.h>

class CTModule {
public:
    // Initializes the CT module on a specified pin with a calibration factor
    static void begin(int ctPin, float calibration);

    // Updates sensor readings and performs calculations
    static void update();

    // Returns the measured RMS current in Amperes
    static float getCurrent();

    // Checks if the CT module is connected and working
    static bool isConnected();

    // Calibrates the sensor with a known current
    static void calibrate(float knownCurrent);

private:
    static float getRawRMS();
    static int setupNoLoadOffset();
    static int _ctPin;
    static float _calibration;
    static float _rmsCurrent;
    static bool _isConnected;
    static int _noLoadOffset;
};

#endif // CT_MODULE_H