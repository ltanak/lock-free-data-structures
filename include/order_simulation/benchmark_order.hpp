#pragma once
#include <chrono>
#include <iostream>
#include <sstream>

/**
 * OrderType struct
 * Indicates different orders
 * @note CANCEL is not currently being used
 */

enum class OrderType {
    BUY,
    SELL,
    CANCEL
};

/**
 * BenchmarkOrder struct
 * The struct that is enqueued and dequeued from the data structures
 * @note need to evaluate the effect of alignas on the struct
 */

struct alignas(64) BenchmarkOrder {
    uint64_t order_id; // 64 bits = 8 bytes (offset starts at 0)
    OrderType type; // int = 4 bytes (starts at 8)
    double price; // 64 bits = 8 bytes (starts at 12)
    uint32_t quantity; // 4 bytes (starts at 20)
    uint64_t timestamp; // 8 bytes (starts)
    uint64_t sequence_number; // 8 bytes = TOTAL = 40 bytes (currently)

    // function to print out the order information
    friend std::ostream& operator<<(std::ostream& os, const BenchmarkOrder &o);
};
