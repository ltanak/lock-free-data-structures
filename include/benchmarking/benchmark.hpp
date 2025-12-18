#pragma once

#include <random>
#include <optional>
#include <atomic>
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"

template<typename DataStructure, typename TOrder>
class BenchmarkWrapper {
public:
    ~BenchmarkWrapper();
    BenchmarkWrapper(DataStructure &structure, TestParams &params);

    auto addThread() -> int;
    auto enqueue_order(TOrder &o, int threadId) -> bool;
    auto dequeue_order(TOrder &o, int threadId) -> bool;
    auto processLatencies() -> void;



private:
    const uint64_t TOTAL_ORDERS_;
    const uint64_t NUM_THREADS_;
    const uint64_t THREAD_LIMIT_;

    DataStructure& structure_;
    std::atomic<int> nextThreadId_{0};
    uint64_t* latencies_enqueue; // contiguous allocation for enqueue
    uint64_t* latencies_dequeue;
    std::vector<uint64_t> localIndex_; // per-thread counters, initialized to 0
};