#pragma once
#include "interfaces/ISensor.hpp"

class SimulatedSensor : public ISensor {
public:
    void inject(bool reading) { _state = reading; }
    bool detect() override { return _state; }

private:
    bool _state = false;
};
