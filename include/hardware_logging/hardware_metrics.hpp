#pragma once
#include <cstdint>

/**
 * Per-thread hardwaremetrics structs to store collected information
 */
struct alignas(64) HardwareMetrics {
    // the different hardware metrics that are currently being monitored
    double cycles = 0.0;
    double instructions = 0.0;
    double cache_refs = 0.0;
    double cache_misses = 0.0;
    double branch_insts = 0.0;
    double branch_misses = 0.0;

    // calculation functions
    double ipc() const;
    double cache_miss_ratio() const;
    double branch_miss_ratio() const;
};
