#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"
#include "scenarios/testing_stress_single_producer.hpp"
#include "scenarios/testing_stress_multi_producer.hpp"
#include "scenarios/testing_order_single_consumer.hpp"
#include "scenarios/testing_order_multi_consumer.hpp"

// Data structure includes
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"

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

    std::string mode = argv[1];
    int test_type = ((std::string)argv[2] == "-s") ? 0 : 1;

    // add code here where you can change the appropriate data structure to use
    // RegularQueue<Order> queue;
    MCConcurrentQueue<Order> queue;
    
    if (mode == "stress") {
        switch (test_type){
            case 0:
                singleProducerStressTest(queue);
                break;
            case 1:
                multiProducerStressTest(queue);
                break;
        }
    } else if (mode == "order") {
        switch (test_type){
            case 0:
                singleConsumerOrderTest(queue);
                break;
            case 1:
                multiConsumerOrderTest(queue);
                break;
        }
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }

    return 0;
}

/**
 * TO DO
 * 
 * fix the interfaces for front() / find out the best way around this: std::queue uses void dequeue()
 * find way where i can collect results - can't just be doing std::cout -> see if i can use message passing / something low-lantency
   to keep track of the pointers to all of the orders
 * start looking into benchmarking library design -> wrapper around functions? use atomic instructions then at end let it compute all the stuff?
   should I have a function of .calculate() which writes all results into a csv, so then i can also use python to graph out the results nicely?
 * automated scripts - how is it done in industry? should i use yaml and then you make the code read the yaml you want? Set parameters like order limit,
   thread count, etc
 * get error logging sorted, so that I can use it throughout
 * write unit tests that make sure the outputs are correct for each of the corresponding data structures and their functions
 */