#pragma once

#include <stdint.h>
#include <thread>
#include <chrono>

namespace lTime {
    auto rdtscp_inline() -> uint64_t;
    auto measure_tsc_ghz() -> double;
}