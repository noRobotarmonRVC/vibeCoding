#pragma once
#include "domain/Direction.hpp"

class IMotorController {
public:
    virtual ~IMotorController() = default;
    virtual void move(Direction direction) = 0;
};
