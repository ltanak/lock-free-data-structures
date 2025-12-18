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
    auto addDequeueThread() -> int;
    auto enqueue_order(TOrder &o, int threadId) -> bool;
    auto dequeue_order(TOrder &o, int threadId) -> bool;
    auto processLatencies() -> void;

private:
    const uint64_t TOTAL_ORDERS_;
    const uint64_t NUM_THREADS_;
    const uint64_t THREAD_LIMIT_;

    DataStructure& structure_;
    std::atomic<int> enqueueThreadId_{0};
    std::atomic<int> dequeueThreadId_{0};
    uint64_t* latencies_enqueue; // contiguous allocation for enqueue
    uint64_t* latencies_dequeue;
    // TOrder** ordering_dequeue; // array of pointers to the dequeued orders
    std::vector<uint64_t> localIndexEnq_; // per-producer counters
    std::vector<uint64_t> localIndexDeq_; // per-consumer counters
};