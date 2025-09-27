
// EnergyMeterModule.h

#ifndef ENERGYMETER_MODULE_H
#define ENERGYMETER_MODULE_H

#include "ACS712.h"

namespace EnergyMeterModule {
    // Public functions for the main application to use
    void begin(int acs712Pin, float voltageCalibration, float sensitivity);
    void update();
    bool isConnected();
    float getPower();
    float getCumulativeEnergy();
    float getPeakPower();
    float getCurrent();
}

#endif // ENERGYMETER_MODULE_H
