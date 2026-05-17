#include "ui/ConsoleDisplay.hpp"
#include <iostream>

void ConsoleDisplay::render(Direction dir, CleanPower power) {
    std::cout << "[RVC] Dir=" << toString(dir)
              << " | Clean=" << toString(power) << "\n";
}
