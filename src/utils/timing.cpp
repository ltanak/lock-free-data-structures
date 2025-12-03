#include "utils/timing.hpp"
#include <stdint.h>

auto rdtscp_inline() -> uint64_t {
    unsigned int aux;
    uint64_t r;
    asm volatile ("rdtscp" : "=a"(r) : : "rcx", "rdx");
    return r;
}