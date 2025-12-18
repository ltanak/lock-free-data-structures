#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace latencies {

    auto getPath() -> std::filesystem::path;
    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    auto createFileName() -> std::string;
    auto write_csv(const std::vector<double>& vec) -> bool;
    auto read_csv() -> bool;

}