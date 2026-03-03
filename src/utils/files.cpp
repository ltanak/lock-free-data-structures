#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "utils/files.hpp"

namespace {

    auto getTime() -> std::string {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%m_%d_%Y_%H_%M_%S");
        return oss.str();
    }
}

namespace latencies {

    auto getPath() -> std::filesystem::path {
        return std::filesystem::path(PROJECT_SOURCE_DIR) / "results/stress_testing/latencies/";
    }

    auto createFileName(const std::string& run_id) -> std::string {
        std::string str = "latencies_";
        if (!run_id.empty()) {
            str += run_id + "_";
        }
        str += getTime() + ".csv";
        return str;
    }

    bool write(const std::vector<double>& enqueue_vec, const std::vector<double>& dequeue_vec, const std::string& run_id) {
        namespace fs = std::filesystem;

        fs::path dirPath = getPath();
        fs::create_directories(dirPath);  // Ensure directory exists
        fs::path filePath = dirPath / createFileName(run_id);
    
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

    bool readCsv(){
        return false;
    }
}

namespace ordering {

    auto getPath() -> std::filesystem::path {
        return std::filesystem::path(PROJECT_SOURCE_DIR) / "results/order_testing/ordering/";
    }

    auto createFileName(const std::string& run_id) -> std::string {
        std::string str = "ordering_";
        if (!run_id.empty()) {
            str += run_id + "_";
        }
        str += getTime() + ".csv";
        return str;
    }

    bool write(const vector<uint64_t>& expected_order, const vector<uint64_t>& actual_order, const std::string& run_id) {
        namespace fs = std::filesystem;

        fs::path dirPath = getPath();
        fs::create_directories(dirPath);  // Ensure directory exists
        fs::path filePath = dirPath / createFileName(run_id);
        if (!fs::exists(filePath)) {
            std::ofstream(filePath) << "expected_id,actual_id\n"; // header
        }
    
        // Open for append
        std::ofstream out(filePath, std::ios::app);

        for (std::size_t i = 0; i < expected_order.size(); ++i){
            out << expected_order[i] << "," << actual_order[i] << "\n";
        }
        return true;
    }

    bool readCsv(){
        return false;
    }
}

namespace exchange {

    auto getPath() -> filesystem::path {
        return std::filesystem::path(PROJECT_SOURCE_DIR) / "results/order_testing/exchange/";
    }

    auto createFileName(const string& run_id) -> string {
        std::string str = "matching_";
        if (!run_id.empty()) {
            str += run_id + "_";
        }
        str += getTime() + ".csv";
        return str;
    }

    auto write(const vector<TradesCycle> expected, const vector<TradesCycle> actual, const std::string& run_id) -> bool {
        namespace fs = std::filesystem;
        
        fs::path dirPath = getPath();
        fs::create_directories(dirPath);  // Ensure directory exists
        fs::path filePath = dirPath / createFileName(run_id);

        if (!fs::exists(filePath)) {
            std::ofstream(filePath) << "cycle,exp_prices,exp_qtities,cycle2,acc_prices,acc_qtities\n"; // header
        }
        
        // open for append
        std::ofstream out(filePath, std::ios::app);

        // code to add here
        for (std::size_t i = 0; i < expected.size(); ++i){
            out << expected[i].cycle << ",";
            for (std::size_t j = 0; j < expected[i].prices.size(); ++j){
                out << expected[i].prices[j];
                if (j < expected[i].prices.size() - 1) {
                    out << "|";
                }
            }
            out << ",";
            for (std::size_t j = 0; j < expected[i].quantities.size(); ++j){
                out << expected[i].quantities[j];
                if (j < expected[i].quantities.size() - 1) {
                    out << "|";
                }
            }
            out << ",";


            // actual ones
            out << actual[i].cycle << ",";
            for (std::size_t j = 0; j < actual[i].prices.size(); ++j){
                out << actual[i].prices[j];
                if (j < actual[i].prices.size() - 1) {
                    out << "|";
                }
            }
            out << ",";
            for (std::size_t j = 0; j < actual[i].quantities.size(); ++j){
                out << actual[i].quantities[j];
                if (j < actual[i].quantities.size() - 1) {
                    out << "|";
                }
            }
            out << "\n";
        }
        
        out.flush();
        out.close();
        return true;
    }
}

namespace hardware {

    auto getPath(const std::string& test_type) -> std::filesystem::path {
        if (test_type == "order") {
            return std::filesystem::path(PROJECT_SOURCE_DIR) / "results/order_testing/hardware/";
        }
        // Default to stress testing
        return std::filesystem::path(PROJECT_SOURCE_DIR) / "results/stress_testing/hardware/";
    }

    auto createFileName(const std::string& run_id) -> std::string {
        std::string str = "hardware_";
        if (!run_id.empty()) {
            str += run_id + "_";
        }
        str += getTime() + ".csv";
        return str;
    }

    auto write(const HardwareMetrics& metrics, const string& run_id, const string& test_type) -> bool {
        namespace fs = std::filesystem;
        
        fs::path dirPath = getPath(test_type);
        fs::create_directories(dirPath);  // Ensure directory exists
        fs::path filePath = dirPath / createFileName(run_id);

        if (!fs::exists(filePath)) {
            std::ofstream(filePath) << "cpu_cycles,cpu_insts,cache_refs,cache_misses,branch_insts,branch_misses\n"; // header
        }

        std::ofstream out(filePath, std::ios::app);
        out << std::fixed << std::setprecision(6);
        out << metrics.cycles << "," 
            << metrics.instructions << ","
            << metrics.cache_refs << ","
            << metrics.cache_misses << ","
            << metrics.branch_insts << ","
            << metrics.branch_misses << "\n";
        return true;
    }
}
