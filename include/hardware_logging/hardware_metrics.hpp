#pragma once

#include <cstdint>

struct alignas(64) HardwareMetrics {
    uint64_t cycles;
    uint64_t instructions;
    uint64_t cache_refs;
    uint64_t cache_misses;
    uint64_t branch_insts;
    uint64_t branch_misses;

    double ipc() const;
    double cache_miss_ratio() const;
    double branch_miss_ratio() const;
};