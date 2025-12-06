#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "utils/files.hpp"

namespace latencies {

    std::filesystem::path getPath() {
        return std::filesystem::path(PROJECT_SOURCE_DIR) / "src/benchmarking/csvs/latencies/";
    }


    auto createFileName() -> std::string {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
        auto str = "latencies_" + oss.str() + ".csv";
        return str;
    }

    bool write_csv(const std::vector<double>& vec) {
        namespace fs = std::filesystem;

        fs::path dirPath = getPath();
        fs::path filePath = dirPath / createFileName();
    
        if (!fs::exists(filePath)) {
            std::ofstream(filePath) << "latencies\n"; // header
        }
    
        // Open for append
        std::ofstream out(filePath, std::ios::app);
    
        for (auto v : vec) {
            out << v << "\n";
        }
        return true;
    }

    bool read_csv(){
        return false;
    }
}