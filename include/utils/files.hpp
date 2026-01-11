#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#include "exchange/book_order.hpp"
#include "exchange/trades_cycle.hpp"
using namespace std;

/**
 * Namespaces for different types of csv writing
 * Each namespace creates and writes the csv file of the results after a test
 * latencies - stress testing
 * ordering - order testing
 * exchange - order testing (downstream effects)
 */

namespace latencies {

    auto getPath() -> filesystem::path;

    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    auto createFileName() -> string;
    auto write(const vector<double>& enqueue_vec, const vector<double>& dequeue_vec) -> bool;
}

namespace ordering {

    auto getPath() -> filesystem::path;

    // "orders_DD_MM_YYYY_HH_MM_SS.csv"
    auto createFileName() -> string;
    auto write(const vector<uint64_t>& expected_order, const vector<uint64_t>& actual_order) -> bool;
}

namespace exchange {

    auto getPath() -> filesystem::path;

    // "matching_DD_MM_YYYY_HH_MM_SS.csv"
    auto createFileName() -> string;
    auto write(const vector<TradesCycle> expected, const vector<TradesCycle> actual) -> bool;
}