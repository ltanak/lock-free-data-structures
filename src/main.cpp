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
 * "stress" - stress testing
 *  -m flag means multi producer stress testing
 *  -s flag means single producer stress testing
 * "order" - order testing
 *  -m flag means multi consumer order testing
 *  -s flag means single consumer order testing
 * 
 * @example
 * ./run.sh stress -s
 * ./run.sh order -m
 * If no arguments are provided, defaults to stress -s
 * 
 * @note Future work will add additional parameters that will be used in testing e.g. order limits
 */

int main(int argc, char* argv[]) {
    TestParams params;
    parseArgs(argc, argv, params);

    // add code here where you can change the appropriate data structure to use
    // RegularQueue<Order> queue;

    // NEED TO MAKE BENCHMARK CODE SUPPORT ANY SPECIFIC LFDS, AS CURRENTLY IT WILL ONLY WORK FOR REGULAR QUEUES
    MCLockFreeQueue<Order> queue;
    BenchmarkWrapper<MCLockFreeQueue<Order>, Order> wrapper(queue, params);

    switch (params.test){
        case TestType::STRESS: stressTest(wrapper, params); break;
        case TestType::ORDER:  orderTest(queue, params); break;
        default: {
            std::cerr << "Unknown mode" << std::endl;
            return 1;
            break;
        }
    }

    return 0;
}

/**
 * get error logging sorted, so that I can use it throughout
 * write unit tests that make sure the outputs are correct for each of the corresponding data structures and their functions
 */