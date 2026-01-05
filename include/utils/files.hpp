#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#include "exchange/book_order.hpp"
#include "exchange/trades_cycle.hpp"

using namespace std;

namespace latencies {

    auto getPath() -> filesystem::path;
    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    auto createFileName() -> string;
    auto writeCsvLatencies(const vector<double>& enqueue_vec, const vector<double>& dequeue_vec) -> bool;
    auto readCsv() -> bool;

}

namespace ordering {

    auto getPath() -> filesystem::path;

    // "orders_DD_MM_YYYY_HH_MM_SS.csv"
    auto createFileName() -> string;
    auto writeCsvOrdering(const vector<uint64_t>& expected_order, const vector<uint64_t>& actual_order) -> bool;
    auto readCsv() -> bool;

}

namespace exchange {

    auto getPath() -> filesystem::path;

    auto createFileName() -> string;
    auto initialise(std::string) -> bool;
    auto write(const vector<TradesCycle> expected, const vector<TradesCycle> actual) -> bool;
}