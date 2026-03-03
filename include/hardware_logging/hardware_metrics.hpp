#pragma once

#include <cstdint>

struct alignas(64) HardwareMetrics {
    uint64_t cycles = 0;
    uint64_t instructions = 0;
    uint64_t cache_refs = 0;
    uint64_t cache_misses = 0;
    uint64_t branch_insts = 0;
    uint64_t branch_misses = 0;

    double ipc() const;
    double cache_miss_ratio() const;
    double branch_miss_ratio() const;
};