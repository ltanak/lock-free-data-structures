#include "utils/timing.hpp"
#include <stdint.h>
#include <chrono>
#include <thread>

namespace ltime {
    
    double measure_tsc_ghz() {
        using clock = std::chrono::high_resolution_clock;
    
        uint64_t t0 = rdtsc_lfence();
        auto s0 = clock::now();
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        uint64_t t1 = rdtsc_lfence();
        auto s1 = clock::now();
    
        double ns = std::chrono::duration<double, std::nano>(s1 - s0).count();
        double cycles = double(t1 - t0);
    
        return cycles / ns;   // cycles per nanosecond
    }
}
