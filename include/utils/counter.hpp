#pragma once
#include <stdint.h>
#include <thread>
#include <chrono>

namespace lcounter {
#ifdef __ARM_ARCH_ISA_A64
    inline auto rdpmc(int counter_id) -> uint64_t {
        uint64_t val;
        asm volatile ("isb; mrs %0, pmccntr_el0" : "=r"(val));
        return val;
    }
#else
    inline auto rdpmc(int counter_id) -> uint64_t {
        uint64_t lo, hi, counter;
        asm volatile("rdpmc" : "=a"(lo), "=d"(hi) : "c"(counter_id));
        return (hi << 32) | lo;
    }
#endif
}
