#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>

#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Scenario Includes
#include "scenarios/testing_stress.hpp"
#include "scenarios/testing_order.hpp"
#include "scenarios/test_inputs.hpp"

// Data structure includes
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "data_structures/queues/rigtorp_mpmc_queue.hpp"
#include "data_structures/ring_buffers/wilt_mpmc_blocking_ring.hpp"
#include "data_structures/ring_buffers/wilt_mpmc_nonblock_ring.hpp"

// Benchmarking harness
#include "benchmarking/benchmark.hpp"

/**
 * @brief Entry point for the lock-free data-structure benchmarking framework.
 *
 * Exactly ONE data-structure block below should be uncommented at a time. To
 * evaluate a different data structure, comment out the currently active block
 * and uncomment the desired one, then rebuild with ./compile.sh.
 *
 * Arguments (parsed by parseArgs):
 *   <mode>    "stress" | "order"     - test scenario to execute
 *   <threads> unsigned int           - producer/consumer thread count
 *                                      (use 1 for SPSC-only structures)
 *   <orders>  unsigned int           - total number of orders to drive through
 *
 * Typical invocation (via run.sh):
 *   ./run.sh --all 4 1000
 *   ./run.sh stress 1 100000000
 *   ./run.sh order 4 1000
 */
int main(int argc, char* argv[]) {
    TestParams params;
    parseArgs(argc, argv, params);

    // Ring-buffer capacity must absorb producer bursts when consumers fall
    // behind. Scales with workload, with a 100k floor for small runs.
    const size_t ring_capacity = std::max(
        static_cast<size_t>(100000),
        params.total_orders * params.thread_count * 2
    );

    // =========================================================================
    // DATA STRUCTURE SELECTION
    // -------------------------------------------------------------------------
    // Uncomment EXACTLY ONE of the six blocks below to select the data
    // structure under test. All blocks construct a `wrapper` variable that is
    // then passed into the scenario dispatcher.
    // =========================================================================

    // -------------------------------------------------------------------------
    // [1] Regular (lock-based) queue - baseline for comparison
    // -------------------------------------------------------------------------
    // RegularQueue<BenchmarkOrder> queue;
    // BenchmarkWrapper<RegularQueue<BenchmarkOrder>, BenchmarkOrder> wrapper(queue, params);
    // std::cout << "[DS] RegularQueue (lock-based baseline)" << std::endl;

    // -------------------------------------------------------------------------
    // [2] moodycamel ConcurrentQueue (MPMC, lock-free)
    // -------------------------------------------------------------------------
    // MCConcurrentQueue<BenchmarkOrder> queue;
    // BenchmarkWrapper<MCConcurrentQueue<BenchmarkOrder>, BenchmarkOrder> wrapper(queue, params);
    // std::cout << "[DS] moodycamel ConcurrentQueue (MPMC)" << std::endl;

    // -------------------------------------------------------------------------
    // [3] moodycamel ReaderWriterQueue (SPSC, lock-free) - use threads=1
    // -------------------------------------------------------------------------
    // MCLockFreeQueue<BenchmarkOrder> queue;
    // BenchmarkWrapper<MCLockFreeQueue<BenchmarkOrder>, BenchmarkOrder> wrapper(queue, params);
    // std::cout << "[DS] moodycamel ReaderWriterQueue (SPSC)" << std::endl;

    // -------------------------------------------------------------------------
    // [4] Wilt MPMC Blocking Ring Buffer
    // -------------------------------------------------------------------------
    // WiltMPMCBlockRing<BenchmarkOrder> ring(ring_capacity);
    // BenchmarkWrapper<WiltMPMCBlockRing<BenchmarkOrder>, BenchmarkOrder> wrapper(ring, params);
    // std::cout << "[DS] Wilt MPMC Blocking Ring" << std::endl;

    // -------------------------------------------------------------------------
    // [5] Wilt MPMC Non-blocking Ring Buffer
    // -------------------------------------------------------------------------
    // WiltMPMCNonBlockRing<BenchmarkOrder> ring(ring_capacity);
    // BenchmarkWrapper<WiltMPMCNonBlockRing<BenchmarkOrder>, BenchmarkOrder> wrapper(ring, params);
    // std::cout << "[DS] Wilt MPMC Non-blocking Ring" << std::endl;

    // -------------------------------------------------------------------------
    // [6] Rigtorp MPMC Queue (default active selection)
    // -------------------------------------------------------------------------
    RigtorpMPMCQueue<BenchmarkOrder> queue(ring_capacity);
    BenchmarkWrapper<RigtorpMPMCQueue<BenchmarkOrder>, BenchmarkOrder> wrapper(queue, params);
    std::cout << "[DS] Rigtorp MPMC Queue" << std::endl;

    // =========================================================================
    // SCENARIO DISPATCH
    // =========================================================================
    switch (params.test) {
        case TestType::STRESS: stressTest(wrapper, params); break;
        case TestType::ORDER:  orderTest(wrapper, params);  break;
        default:
            std::cerr << "Unknown mode" << std::endl;
            return 1;
    }
    return 0;
}
