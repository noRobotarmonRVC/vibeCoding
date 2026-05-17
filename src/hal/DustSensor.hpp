#pragma once
#include "interfaces/ISensor.hpp"

class DustSensor : public ISensor {
public:
    bool detect() override;
};
