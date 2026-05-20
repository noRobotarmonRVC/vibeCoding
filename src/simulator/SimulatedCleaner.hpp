#pragma once
#include <vector>
#include "interfaces/ICleanerController.hpp"

class SimulatedCleaner : public ICleanerController {
public:
    void setPower(CleanPower power) override {
        _last = power;
        _log.push_back(power);
    }

    [[nodiscard]] CleanPower last() const { return _last; }
    [[nodiscard]] const std::vector<CleanPower>& log() const { return _log; }
    void clearLog() { _log.clear(); }

private:
    CleanPower _last = CleanPower::OFF;
    std::vector<CleanPower> _log;
};
