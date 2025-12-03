#include "scenarios/test_inputs.hpp"
#include <iostream>
#include <sstream>

void parseArgs(int argc, char* argv[], TestParams& params){
    try {
        std::string mode = argv[1];
        params.test = (mode == "stress") ? TestType::STRESS : TestType::ORDER;
        params.thread_count = std::stoi(argv[2]);
        params.thread_order_limit = std::stoi(argv[3]);
        params.total_orders = params.thread_count * params.thread_order_limit;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::exit(1);
    }
}