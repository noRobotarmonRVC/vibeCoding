#pragma once
#include <cstdint>

enum class RvcState : std::uint8_t {
    IDLE,
    CLEANING,
    AVOIDING_OBSTACLE,
    ESCAPING,
    INTENSIFYING
};

inline const char* toString(RvcState s) {
    switch (s) {
        case RvcState::IDLE:              return "IDLE";
        case RvcState::CLEANING:          return "CLEANING";
        case RvcState::AVOIDING_OBSTACLE: return "AVOIDING_OBSTACLE";
        case RvcState::ESCAPING:          return "ESCAPING";
        case RvcState::INTENSIFYING:      return "INTENSIFYING";
    }
    return "UNKNOWN";
}
