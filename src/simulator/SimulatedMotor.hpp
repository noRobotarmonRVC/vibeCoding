#pragma once
#include <vector>
#include "interfaces/IMotorController.hpp"

class SimulatedMotor : public IMotorController {
public:
    void move(Direction direction) override {
        _last = direction;
        _log.push_back(direction);
    }

    [[nodiscard]] Direction last() const { return _last; }
    [[nodiscard]] const std::vector<Direction>& log() const { return _log; }
    void clearLog() { _log.clear(); }

private:
    Direction _last = Direction::STOP;
    std::vector<Direction> _log;
};
