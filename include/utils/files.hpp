#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#include "exchange/book_order.hpp"
#include "exchange/trades_cycle.hpp"
#include "hardware_logging/hardware_metrics.hpp"
using namespace std;

/**
 * Namespaces for different types of csv writing
 * Each namespace creates and writes the csv file of the results after a test
 * 
 * Directory structure:
 * - stress_testing: latencies and hardware (stress testing results)
 * - order_testing: ordering, exchange and hardware (order testing results)
 * 
 * latencies - latency measurements during stress testing
 * hardware - hardware counter data during stress testing
 * ordering - order preservation testing during order testing
 * exchange - matching engine results during order testing
 */

namespace latencies {

    auto getPath() -> filesystem::path;

    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    auto createFileName(const string& run_id) -> string;
    auto write(const vector<double>& enqueue_vec, const vector<double>& dequeue_vec, const string& run_id) -> bool;
}

namespace ordering {

    auto getPath() -> filesystem::path;

    // "orders_DD_MM_YYYY_HH_MM_SS.csv"
    auto createFileName(const string& run_id) -> string;
    auto write(const vector<uint64_t>& expected_order, const vector<uint64_t>& actual_order, const string& run_id) -> bool;
}

namespace exchange {

    auto getPath() -> filesystem::path;

    // "matching_DD_MM_YYYY_HH_MM_SS.csv"
    auto createFileName(const string& run_id) -> string;
    auto write(const vector<TradesCycle> expected, const vector<TradesCycle> actual, const string& run_id) -> bool;
}

namespace hardware {

    // test_type "stress" or "order" determines subdirectory
    auto getPath(const string& test_type = "stress") -> filesystem::path;

    auto createFileName(const string& run_id) -> string;
    auto write(const HardwareMetrics& metrics, const string& run_id, const string& test_type = "stress") -> bool;
}