#include "domain/DefaultNavigationStrategy.hpp"

Direction DefaultNavigationStrategy::navigate(const SensorData& data) {
    if (data.is_front_blocked && data.is_left_blocked && data.is_right_blocked) {
        return Direction::BACKWARD;
    }
    if (data.is_front_blocked && !data.is_left_blocked) {
        return Direction::LEFT;
    }
    if (data.is_front_blocked && !data.is_right_blocked) {
        return Direction::RIGHT;
    }
    return Direction::FORWARD;
}
