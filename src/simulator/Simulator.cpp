#include "simulator/Simulator.hpp"

Simulator::Simulator()
    : _controller(&_front, &_left, &_right, &_dust, &_motor, &_cleaner, &_nav) {}

void Simulator::start() { _controller.start(); }
void Simulator::stop()  { _controller.stop(); }

void Simulator::tick() {
    _controller.onTick();
}

void Simulator::triggerFrontObstacle() {
    _controller.onFrontObstacleDetected();
}

void Simulator::injectFront(bool reading) { _front.inject(reading); }
void Simulator::injectLeft(bool reading)  { _left.inject(reading); }
void Simulator::injectRight(bool reading) { _right.inject(reading); }
void Simulator::injectDust(bool reading)  { _dust.inject(reading); }

Direction  Simulator::lastDirection() const { return _motor.last(); }
CleanPower Simulator::lastPower()     const { return _cleaner.last(); }

const std::vector<Direction>&  Simulator::motorLog()   const { return _motor.log(); }
const std::vector<CleanPower>& Simulator::cleanerLog() const { return _cleaner.log(); }
