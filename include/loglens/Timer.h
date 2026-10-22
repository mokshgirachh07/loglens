#pragma once

#include <chrono>

namespace loglens {

class Timer {
public:
    Timer() : start_(Clock::now()) {}

    double seconds() const {
        return std::chrono::duration<double>(Clock::now() - start_).count();
    }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point start_;
};

} 