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
    
    if (mode == "stress") {
        switch (test_type){
            case 0:
                std::cout << "Running single producer stress test: " << std::endl;
                singleProducerStressTest();
                break;
            case 1:
                std::cout << "Running multi producer stress test: " << std::endl;
                multiProducerStressTest();
                break;
        }
    } else if (mode == "order") {
        switch (test_type){
            case 0:
                std::cout << "Running single consumer order test: " << std::endl;
                singleConsumerOrderTest();
                break;
            case 1:
                std::cout << "Running multi consumer order test: " << std::endl;
                multiConsumerOrderTest();
                break;
        }
    } else {
        std::cerr << "Unknown mode: " << mode << std::endl;
        return 1;
    }


    /**
     * TO IMPLEMENT NEXT
     * Thread that controls the market state
     * Once done need to then just print and output results and do a quick measure on the speed of transactions
     * Create "framework" / section where we will put the endpoint of the lock-free data structure to start getting ready for that
     */

    return 0;
}
