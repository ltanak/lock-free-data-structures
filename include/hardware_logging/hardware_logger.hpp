#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <mutex>

#include "hardware_logging/hardware_metrics.hpp"

class HardwareLogger {

public:
    HardwareLogger();

    // register thread's values
    auto registerThreadMetrics(int tid, const HardwareMetrics &metrics) -> void;
    auto getMetrics() -> HardwareMetrics;
    auto getListMetrics() -> std::vector<HardwareMetrics>;
    auto clear() -> void;

private:
    mutable std::mutex metrics_lock_;
    std::vector<HardwareMetrics> thread_metrics_;
};