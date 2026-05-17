#include "hal/FrontSensor.hpp"

bool FrontSensor::detect() {
    bool val = _triggered;
    _triggered = false;
    return val;
}

void FrontSensor::onInterrupt() {
    _triggered = true;
}
