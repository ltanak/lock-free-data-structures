#pragma once

#include <stdint.h>
#include <thread>
#include <chrono>

uint64_t rdtscp_inline();

double measure_tsc_ghz();