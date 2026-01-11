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

struct BenchmarkOrder {
    uint64_t order_id;
    OrderType type;
    double price;
    uint32_t quantity;
    uint64_t timestamp;
    uint64_t sequence_number;

    // function to print out the order information
    friend std::ostream& operator<<(std::ostream& os, const BenchmarkOrder &o);
};
