#pragma once
#include "interfaces/ISensor.hpp"

class FrontSensor : public ISensor {
public:
    bool detect() override;
    void onInterrupt();

private:
    bool _triggered = false;
};
