#pragma once

#include <cstdint>

struct alignas(64) HardwareMetrics {
    double cycles = 0.0;
    double instructions = 0.0;
    double cache_refs = 0.0;
    double cache_misses = 0.0;
    double branch_insts = 0.0;
    double branch_misses = 0.0;

    double ipc() const;
    double cache_miss_ratio() const;
    double branch_miss_ratio() const;
};