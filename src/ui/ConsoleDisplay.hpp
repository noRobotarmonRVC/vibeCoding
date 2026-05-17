#pragma once
#include "domain/RvcState.hpp"
#include "domain/Direction.hpp"
#include "domain/CleanPower.hpp"

class ConsoleDisplay {
public:
    void render(Direction dir, CleanPower power);
};
