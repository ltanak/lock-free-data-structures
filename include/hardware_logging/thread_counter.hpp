#pragma once

#include "hardware_logging/hardware_metrics.hpp"

#include <cstdint>
#include <atomic>
#include <memory>
#include <vector>
#include <array>
#include <mutex>

class ThreadHardwareCounter {

public: 
    ThreadHardwareCounter();
    ~ThreadHardwareCounter();

    bool setup(); // the setup for perf required
    void start();
    void stop();
    void clear();

    HardwareMetrics snapshot();

private:
    static constexpr size_t HW_COUNTERS = 6;
    std::array<int, HW_COUNTERS> fds_; // file descriptors
    std::array<uint64_t, HW_COUNTERS> initial_;
    std::array<uint64_t, HW_COUNTERS> accumulated_;
    bool initialised_;
};