#pragma once
#include "simulator/SimulatedSensor.hpp"
#include "simulator/SimulatedMotor.hpp"
#include "simulator/SimulatedCleaner.hpp"
#include "domain/DefaultNavigationStrategy.hpp"
#include "app/RvcController.hpp"

class Simulator {
public:
    Simulator();

    void start();
    void stop();
    void tick();
    void triggerFrontObstacle();

    void injectFront(bool reading);
    void injectLeft(bool reading);
    void injectRight(bool reading);
    void injectDust(bool reading);

    Direction  lastDirection() const;
    CleanPower lastPower()     const;

    const std::vector<Direction>&  motorLog()   const;
    const std::vector<CleanPower>& cleanerLog() const;

private:
    SimulatedSensor          _front;
    SimulatedSensor          _left;
    SimulatedSensor          _right;
    SimulatedSensor          _dust;
    SimulatedMotor           _motor;
    SimulatedCleaner         _cleaner;
    DefaultNavigationStrategy _nav;
    RvcController            _controller;
};
