#pragma once
#include "domain/CleanPower.hpp"

class ICleanerController {
public:
    virtual ~ICleanerController() = default;
    virtual void setPower(CleanPower power) = 0;
};
