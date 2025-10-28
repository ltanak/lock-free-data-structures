#pragma once
#include <chrono>

enum class OrderType {
    BUY,
    SELL,
    CANCEL
};

struct Order {
    uint64_t order_id;
    OrderType type;
    double price;
    uint32_t quantity;
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
};