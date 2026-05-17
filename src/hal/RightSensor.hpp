#pragma once
#include "interfaces/ISensor.hpp"

class RightSensor : public ISensor {
public:
    bool detect() override;
};
