#pragma once
#include "interfaces/INavigationStrategy.hpp"

class DefaultNavigationStrategy : public INavigationStrategy {
public:
    Direction navigate(const SensorData& data) override;
};
