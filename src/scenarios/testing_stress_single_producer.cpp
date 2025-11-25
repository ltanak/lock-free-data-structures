#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

/**
 * @brief Single producer stress test
 * Purpose of test is to benchmark the throughput of a single producer adding to a data structure
 * A single thread will be generating orders and pushing them to the data structure
 * 
 * Logic for this code has been moved into include/scenarios in corresponding .hpp files
 * This allows for it to be templated correctly, so the data structure only needs to be defined once in main.cpp when benchmarking
 */