#include "utils/timing.hpp"
#include <stdint.h>
#include <chrono>
#include <thread>

namespace ltime {

    // if you inline this, need to add memory clobber to asm
    // investigate the latencies of this before you do it
    uint64_t rdtscp_inline() {
        uint32_t lo, hi;
        asm volatile ("rdtscp" : "=a"(lo), "=d"(hi) :: "rcx");
        return (uint64_t(hi) << 32) | lo;
    }
    
    double measure_tsc_ghz() {
        using clock = std::chrono::high_resolution_clock;
    
        uint64_t t0 = rdtscp_inline();
        auto s0 = clock::now();
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
        uint64_t t1 = rdtscp_inline();
        auto s1 = clock::now();
    
        double ns = std::chrono::duration<double, std::nano>(s1 - s0).count();
        double cycles = double(t1 - t0);
    
        return cycles / ns;   // cycles per nanosecond
    }
}
