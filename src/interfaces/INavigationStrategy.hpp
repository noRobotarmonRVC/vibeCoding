#pragma once
#include "domain/SensorData.hpp"
#include "domain/Direction.hpp"

class INavigationStrategy {
public:
    virtual ~INavigationStrategy() = default;
    virtual Direction navigate(const SensorData& data) = 0;
};
