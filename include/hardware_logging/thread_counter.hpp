#pragma once
#include <cstdint>
#include <atomic>
#include <memory>
#include <vector>
#include <array>
#include <mutex>

#include "hardware_logging/hardware_metrics.hpp"

/**
 * @class ThreadHardwareCounter
 * @brief Per-thread hardware counter class, storing local HardwareMetrics struct
 */
class ThreadHardwareCounter {

public: 
    ThreadHardwareCounter();
    ~ThreadHardwareCounter();

    // initialisation / helper methods
    bool setup();
    void start();
    void stop();
    void clear();

    // returns the collected metrics
    HardwareMetrics snapshot();

private:
    static constexpr size_t HW_COUNTERS = 6;
    std::array<int, HW_COUNTERS> fds_; // file descriptors
    std::array<uint64_t, HW_COUNTERS> initial_;
    std::array<uint64_t, HW_COUNTERS> accumulated_;
    bool initialised_;
};
