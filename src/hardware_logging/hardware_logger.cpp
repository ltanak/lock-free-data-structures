#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <mutex>

#include "hardware_logging/hardware_metrics.hpp"
#include "hardware_logging/hardware_logger.hpp"

HardwareLogger::HardwareLogger() {}

void HardwareLogger::registerThreadMetrics(int tid, const HardwareMetrics &metrics){
    std::lock_guard<std::mutex> lock(metrics_lock_);
    if ((size_t)tid >= thread_metrics_.size()){
        thread_metrics_.resize(tid + 1);
    }
    thread_metrics_[tid] = metrics;
}

HardwareMetrics HardwareLogger::getMetrics() {
    std::lock_guard<std::mutex> lock(metrics_lock_);
    HardwareMetrics agg;

    for (const auto &m : thread_metrics_){
        agg.cycles += m.cycles;
        agg.cache_refs += m.cache_refs;
        agg.cache_misses += m.cache_misses;
        agg.instructions += m.instructions;
        agg.branch_insts += m.branch_insts;
        agg.branch_misses += m.branch_misses;
    }
    return agg;
}

std::vector<HardwareMetrics> HardwareLogger::getListMetrics(){
    std::lock_guard<std::mutex> lock(metrics_lock_);
    return thread_metrics_;
}

void HardwareLogger::clear() {
    std::lock_guard<std::mutex> lock(metrics_lock_);
    thread_metrics_.clear();
}