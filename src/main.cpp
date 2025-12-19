#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Scenario Includes
#include "scenarios/testing_stress.hpp"
#include "scenarios/testing_order.hpp"

// Code inputs
#include "scenarios/test_inputs.hpp"

// Data structure includes
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"

// Benchmark
#include "benchmarking/benchmark.hpp"

/**
 * @brief Main function using arguments passed in will branch to the different tests
 * Arguments:
 _ "stress" - stress testing
 | "order" - order testing
 _ XXXX - number of threads (1 for SPSC style data structures, anything else for MPMC style)
 _ XXXX - number of orders / transactions to execute
 * @example
 * ./run.sh stress 1 100'000'000
 * ./run.sh order 4 1000
 * 
 * @note Future work will add additional parameters that will be used in testing e.g. time based
 */

int main(int argc, char* argv[]) {
    TestParams params;
    parseArgs(argc, argv, params);

    MCConcurrentQueue<Order> queue;
    BenchmarkWrapper<MCConcurrentQueue<Order>, Order> wrapper(queue, params);

    switch (params.test){
        case TestType::STRESS: stressTest(wrapper, params); break;
        case TestType::ORDER:  orderTest(wrapper, params); break;
        default: {
            std::cerr << "Unknown mode" << std::endl;
            return 1;
            break;
        }
    }

    return 0;
}
