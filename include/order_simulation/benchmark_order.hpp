#pragma once
#include <chrono>
#include <iostream>
#include <sstream>

enum class OrderType {
    BUY,
    SELL,
    CANCEL
};

struct BenchmarkOrder {
    uint64_t order_id;
    OrderType type;
    double price;
    double quantity;
    uint64_t timestamp;
    uint64_t sequence_number;

    friend std::ostream& operator<<(std::ostream& os, const BenchmarkOrder &o);
};