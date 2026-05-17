#include <iostream>
#include "simulator/Simulator.hpp"
#include "ui/ConsoleDisplay.hpp"

int main() {
    Simulator sim;
    ConsoleDisplay display;

    std::cout << "=== RVC Control SW Demo ===\n\n";

    sim.start();
    display.render(sim.lastDirection(), sim.lastPower());

    std::cout << "\n-- Normal forward navigation (3 ticks) --\n";
    for (int i = 0; i < 3; ++i) {
        sim.tick();
        display.render(sim.lastDirection(), sim.lastPower());
    }

    std::cout << "\n-- Dust detected (intensify for " << RvcController::INTENSIFY_DURATION << " ticks) --\n";
    sim.injectDust(true);
    sim.tick();
    sim.injectDust(false);
    display.render(sim.lastDirection(), sim.lastPower());
    for (int i = 0; i < RvcController::INTENSIFY_DURATION; ++i) {
        sim.tick();
        display.render(sim.lastDirection(), sim.lastPower());
    }

    std::cout << "\n-- Front obstacle, left side open --\n";
    sim.injectLeft(false);
    sim.injectRight(true);
    sim.triggerFrontObstacle();
    sim.injectRight(false);
    display.render(sim.lastDirection(), sim.lastPower());

    std::cout << "\n-- Surrounded (front + left + right blocked) --\n";
    sim.injectLeft(true);
    sim.injectRight(true);
    sim.triggerFrontObstacle();
    sim.injectLeft(false);
    sim.injectRight(false);
    display.render(sim.lastDirection(), sim.lastPower());

    std::cout << "\n-- Stop --\n";
    sim.stop();
    display.render(sim.lastDirection(), sim.lastPower());

    return 0;
}
