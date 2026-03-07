#pragma once

#include <random>
#include <optional>
#include <atomic>
#include <vector>
#include <string>
#include <mutex>

#include "scenarios/test_inputs.hpp"

#include "utils/timing.hpp"
#include "utils/structs.hpp"

#include "exchange/matching_engine.hpp"

#include "order_simulation/market_state.hpp"
#include "order_simulation/collection_order_generator.hpp"

#include "hardware_logging/hardware_logger.hpp"
#include "hardware_logging/thread_counter.hpp"

/**
 * @class BenchmarkWrapper
 * @brief Benchmarking harness for concurrent data structure evaluation.
 *
 * Orchestrates performance testing of concurrent data-structure implementations
 * across latency, ordering, and exchange matching scenarios. Manages thread
 * coordination, hardware performance counter collection, and result aggregation.
 *
 * **Responsibilities:**
 * - **Thread Management:** Tracks enqueue/dequeue thread IDs for per-thread metrics
 * - **Latency Measurement:** Records nanosecond-precision timestamps for queue operations
 * - **Order Preservation:** Tracks sequence numbers and execution order for correctness
 * - **Exchange Simulation:** Drives matching engine with generated orders and validates outputs
 * - **Hardware Profiling:** Collects CPU cycles, cache misses, branch predictions
 * - **Result Serialisation:** Writes benchmarks results and hardware metrics to CSV
 *
 * **Key Features:**
 * - Template-based generic design supporting any queue implementation and order type
 * - Hardware counter integration via PAPI for low-level performance analysis
 * - Configurable test scenarios (stress, ordering, exchange)
 * - Per-thread metrics aggregation with proper thread safety
 * - Automatic CSV output generation for post-processing and visualization
 *
 * **Output Files:**
 * - Latency CSV: enqueue/dequeue latencies in nanoseconds
 * - Ordering CSV: expected vs actual order sequences
 * - Exchange CSV: matching engine execution traces
 * - Hardware CSV: aggregated per-thread performance counter data
 *
 * @tparam DataStructure The queue implementation being benchmarked
 * @tparam TOrder The order type being processed (always BenchmarkOrder)
 */
template<typename DataStructure, typename TOrder>
class BenchmarkWrapper {
public:

    ~BenchmarkWrapper();
    BenchmarkWrapper(DataStructure &structure, TestParams &params);

    // adds thread IDs for enqueing or dequeing threads
    auto addEnqThread() -> int;
    auto addDeqThread() -> int;

    // operations for enqueing, and the different types of dequeueing
    auto enqueueOrder(TOrder &o, int threadId) -> bool;
    auto dequeueLatency(TOrder &o, int threadId) -> bool;
    auto dequeueOrdering(TOrder &o, int threadId) -> bool;

    auto setLatencyVectors(const std::vector<uint64_t> &enqueue, const std::vector<uint64_t> &dequeue) -> void;
    auto setOrderingVectors(const std::vector<uint64_t> &timestamps, const std::vector<uint64_t> &sequence) -> void;

    // csv writing entry functions
    auto processLatencies() -> void;
    auto processOrders(CollectionOrderGenerator<BenchmarkOrder> &generator, MarketState &market) -> void;
    auto processMatching(std::vector<TOrder>& orders, std::vector<uint64_t>& actual_order) -> void;
    auto processHardwareCounters() -> void;

    // hardware logger functions
    auto getHardwareLogger() -> HardwareLogger&;
    auto getThreadCounter() -> ThreadHardwareCounter&;
    auto aggregateHardwareMetrics(uint64_t num_threads) -> void;

private:
    // cycle count for exchange
    uint64_t current_cycle_ = 0;

    // consts for input parameters
    const uint64_t TOTAL_ORDERS_;
    const uint64_t NUM_THREADS_;
    const uint64_t THREAD_LIMIT_;
    const std::string RUN_ID_;
    const TestType TEST_TYPE_;

    // data structure templated argument
    DataStructure& structure_;

    // atomics for thread IDs
    std::atomic<int> enqueue_thread_id_{0};
    std::atomic<int> dequeue_thread_id{0};

    // stores results of buffers from stress testing
    std::vector<uint64_t> latencies_enqueue_;
    std::vector<uint64_t> latencies_dequeue_;

    // stores results from buffers from order testing
    std::vector<uint64_t> sequence_dequeue_;
    std::vector<uint64_t> timestamps_dequeue_;

    // the matching engine
    MatchingEngine<TOrder> exchange_;

    // hardware logging
    HardwareLogger hw_logger;
    mutable std::mutex hw_lock_;
    std::unordered_map<std::thread::id, std::unique_ptr<ThreadHardwareCounter>, std::hash<std::thread::id>> thread_hw_counters_;
};
