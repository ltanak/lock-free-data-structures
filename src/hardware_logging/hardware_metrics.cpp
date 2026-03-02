#include "hardware_logging/hardware_metrics.hpp"

#include <cstdint>

double HardwareMetrics::ipc() const {
    return (instructions > 0) ? static_cast<double>(instructions) / cycles : 0.0;
}

double HardwareMetrics::cache_miss_ratio() const {
    return (cache_refs > 0) ? static_cast<double>(cache_misses) / cache_refs : 0.0;
}

double HardwareMetrics::branch_miss_ratio() const {
    return (branch_insts > 0) ? static_cast<double>(branch_misses) / branch_insts : 0.0;
}