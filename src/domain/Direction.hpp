#pragma once
#include <cstdint>

enum class Direction : std::uint8_t { FORWARD, BACKWARD, LEFT, RIGHT, STOP };

inline const char* toString(Direction d) {
    switch (d) {
        case Direction::FORWARD:  return "FORWARD";
        case Direction::BACKWARD: return "BACKWARD";
        case Direction::LEFT:     return "LEFT";
        case Direction::RIGHT:    return "RIGHT";
        case Direction::STOP:     return "STOP";
    }
    return "UNKNOWN";
}
