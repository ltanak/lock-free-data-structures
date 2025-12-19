#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

namespace latencies {

    auto getPath() -> filesystem::path;
    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    auto createFileName() -> string;
    auto write_csv_latencies(const vector<double>& enqueue_vec, const vector<double>& dequeue_vec) -> bool;
    auto read_csv() -> bool;

}

namespace ordering {

    auto getPath() -> filesystem::path;

    // "orders_DD_MM_YYYY_HH_MM_SS.csv"
    auto createFileName() -> string;
    auto write_csv_ordering() -> bool;
    auto read_csv() -> bool;

}