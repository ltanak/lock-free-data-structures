#include "scenarios/test_inputs.hpp"
#include <iostream>
#include <sstream>
#include <charconv>
#include <cstring>
#include <string>

void parseArgs(int argc, char* argv[], TestParams& params){
    try {
        std::string mode = argv[1];
        params.test = (mode == "stress") ? TestType::STRESS : TestType::ORDER;
        params.thread_count = std::stoull(argv[2]);
        params.thread_order_limit = std::stoull(argv[3]);
        params.total_orders = params.thread_count * params.thread_order_limit;

        // Optional flags
        for (int i = 4; i < argc; ++i) {
            if (std::strcmp(argv[i], "--seed") == 0)
                std::from_chars(argv[i+1], argv[i+1] + std::strlen(argv[i+1]), params.seed);
        }
        std::cout << "Seed: " << params.seed << std::endl;


    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::exit(1);
    }
}