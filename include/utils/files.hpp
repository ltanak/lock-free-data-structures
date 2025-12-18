#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace latencies {

    auto getPath() -> std::filesystem::path;
    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    auto createFileName() -> std::string;
    auto write_csv_latencies(const std::vector<double>& enqueue_vec, const std::vector<double>& dequeue_vec) -> bool;
    auto read_csv() -> bool;

}