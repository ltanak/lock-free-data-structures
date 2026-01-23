#pragma once

#include <random>
#include <optional>
#include <atomic>
#include <vector>
#include <string>
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"
#include "utils/structs.hpp"
#include "exchange/matching_engine.hpp"

// This will have to either be templ
#include "order_simulation/collection_order_generator.hpp"

template<typename DataStructure, typename TOrder>
class BenchmarkWrapper {
public:

    ~BenchmarkWrapper();
    BenchmarkWrapper(DataStructure &structure, TestParams &params);

    // adds thread IDs for enqueing or dequeing threads
    auto addEnqThread() -> int;
    auto addDeqThread() -> int;

    // pre-processing functions
    auto preprocessEnqueue(TOrder &o, int threadId) -> bool;
    auto preprocessDequeue(TOrder &o, int threadId) -> bool;
    
    // operations for enqueing, and the different types of dequeueing
    auto enqueueOrder(TOrder &o, int threadId) -> bool;
    auto dequeueLatency(TOrder &o, int threadId) -> bool;
    auto dequeueOrdering(TOrder &o, int threadId) -> bool;

    auto setLatencyVectors(const std::vector<uint64_t> &enqueue, const std::vector<uint64_t> &dequeue) -> void;

    // csv writing entry functions
    auto processLatencies() -> void;
    auto processOrders(CollectionOrderGenerator<BenchmarkOrder> &generator) -> void;
    auto processMatching(std::vector<TOrder>& orders, std::vector<uint64_t>& actual_order) -> void;

private:
    // cycle count for exchange
    uint64_t current_cycle_ = 0;

    // consts for input parameters
    const uint64_t TOTAL_ORDERS_;
    const uint64_t NUM_THREADS_;
    const uint64_t THREAD_LIMIT_;

    // data structure templated argument
    DataStructure& structure_;

    // atomics for thread IDs
    std::atomic<int> enqueue_thread_id_{0};
    std::atomic<int> dequeue_thread_id{0};

    // contiguous allocation for latency writing - only used for enqueueOrder & dequeueLatency
    // uint64_t* latencies_enqueue_;
    // uint64_t* latencies_dequeue_;
    std::vector<uint64_t> latencies_enqueue_;
    std::vector<uint64_t> latencies_dequeue_;

    // contiguous allocation for order writing - only used for dequeueOrdering
    uint64_t* sequence_dequeue_;
    uint64_t* timestamps_dequeue_;

    std::vector<uint64_t> local_index_enq_; // per-producer counters
    std::vector<uint64_t> local_index_deq_; // per-consumer counters

    // std::vector<uint64> enqueue_latencies_;
    // std::

    // the matching engine
    MatchingEngine<TOrder> exchange_;
};
