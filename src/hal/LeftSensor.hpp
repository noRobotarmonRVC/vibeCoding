#pragma once
#include "interfaces/ISensor.hpp"

class LeftSensor : public ISensor {
public:
    bool detect() override;
};
