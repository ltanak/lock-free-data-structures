#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

namespace latencies {

    std::filesystem::path getPath();

    // "latencies_DD_MM_YYYY_HH_MM_SS.csv".
    std::string createFileName();

    bool write_csv(const std::vector<double>& vec);

    bool read_csv();

}