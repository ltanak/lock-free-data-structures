#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "utils/files.hpp"

namespace latencies {

    auto getPath() -> std::filesystem::path {
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

    bool write_csv_latencies(const std::vector<double>& enqueue_vec, const std::vector<double>& dequeue_vec) {
        namespace fs = std::filesystem;

        fs::path dirPath = getPath();
        fs::path filePath = dirPath / createFileName();
    
        if (!fs::exists(filePath)) {
            std::ofstream(filePath) << "enqueue_latency_ns,dequeue_latency_ns\n"; // header
        }
    
        // Open for append
        std::ofstream out(filePath, std::ios::app);

        for (std::size_t i = 0; i < enqueue_vec.size(); ++i){
            // out method here is ofstream out
            out << enqueue_vec[i] << "," << dequeue_vec[i] << "\n";
        }
        return true;
    }

    bool read_csv(){
        return false;
    }
}

namespace ordering {

    auto getPath() -> std::filesystem::path {
        return std::filesystem::path(PROJECT_SOURCE_DIR) / "src/benchmarking/csvs/ordering/";
    }

    auto createFileName() -> std::string {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
        auto str = "ordering_" + oss.str() + ".csv";
        return str;
    }

    bool write_csv_ordering(const vector<uint64_t>& expected_order, const vector<uint64_t>& actual_order) {
        namespace fs = std::filesystem;

        fs::path dirPath = getPath();
        fs::path filePath = dirPath / createFileName();
        if (!fs::exists(filePath)) {
            std::ofstream(filePath) << "expected_id,actual_id\n"; // header
        }
    
        // Open for append
        std::ofstream out(filePath, std::ios::app);

        // code to add here:
        for (std::size_t i = 0; i < expected_order.size(); ++i){
            out << expected_order[i] << "," << actual_order[i] << "\n";
        }
        return true;
    }


    bool read_csv(){
        return false;
    }
}
