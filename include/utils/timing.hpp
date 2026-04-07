#pragma once

#include <stdint.h>
#include <thread>
#include <chrono>

/**
 * @brief RDTSC_LFENCE functions for ARM & x86
 */
namespace ltime {
#ifdef __ARM_ARCH_ISA_A64
    inline auto rdtsc_lfence() -> uint64_t {
        uint64_t val;
        asm volatile("isb; mrs %0, cntvct_el0" : "=r"(val) :: "memory");
        return val;
    }
#else
    inline auto rdtsc_lfence() -> uint64_t {
        uint64_t lo, hi, aux;
        asm volatile("lfence; rdtsc" : "=a"(lo), "=d"(hi) :: "memory");
        // return (static_cast<uint64_t>(hi) << 32) | static_cast<uint64_t>(lo);
        return (hi << 32) | lo;
    }
#endif
    auto measure_tsc_ghz() -> double;
}
