#pragma once

#include <stdint.h>
#include <thread>
#include <chrono>

namespace ltime {

    inline auto rdtsc_lfence() -> uint64_t {
        uint32_t lo, hi, aux;
        asm volatile("lfence; rdtsc" : "=a"(lo), "=d"(hi) :: "memory");
        return (hi << 32) | lo;
    }
    
    auto measure_tsc_ghz() -> double;
}
