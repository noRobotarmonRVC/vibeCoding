#pragma once

enum class CleanPower { OFF, ON, POWER_UP };

inline const char* toString(CleanPower p) {
    switch (p) {
        case CleanPower::OFF:      return "OFF";
        case CleanPower::ON:       return "ON";
        case CleanPower::POWER_UP: return "POWER_UP";
    }
    return "UNKNOWN";
}
