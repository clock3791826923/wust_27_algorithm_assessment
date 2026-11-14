#include "solver.hpp"

#include <thread>

using namespace std::chrono_literals;

void solver_loop(const Latest<Target>& latest, const std::atomic<bool>& stop) {
    auto next = Clock::now();
    while (!stop) {
        next += 1ms;
        [[maybe_unused]] Target t = latest.read();
        std::this_thread::sleep_until(next);
    }
}
