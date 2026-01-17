#pragma once

#include <stdint.h>
#include <thread>
#include <chrono>

namespace ltime {
#ifdef __ARM_ARCH_ISA_A64
    inline auto rdtsc_lfence() -> uint64_t {
        uint64_t val;
        asm volatile("isb; mrs %0, cntvct_el0" : "=r"(val) :: "memory");
        return val;
    }
#else
    inline auto rdtsc_lfence() -> uint64_t {
        uint32_t lo, hi, aux;
        asm volatile("lfence; rdtsc" : "=a"(lo), "=d"(hi) :: "memory");
        return (hi << 32) | lo;
    }
#endif
    auto measure_tsc_ghz() -> double;
}
