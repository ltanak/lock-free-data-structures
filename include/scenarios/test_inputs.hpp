#pragma once
#include <chrono>
#include <iostream>
#include <sstream>

// If we want to add different tests in the future
enum class TestType {
    STRESS,
    ORDER
};

struct TestParams {
    TestType test;
    uint64_t thread_count;
    uint64_t thread_order_limit; // Order limit is per-thread order limit, so if we have 4 threads, limit of 1,000, we will execute 4,000 total orders
    uint64_t total_orders; // This will be the result of threads * order_limit

    friend void parseArgs(int argc, char* argv[], TestParams& params);
};