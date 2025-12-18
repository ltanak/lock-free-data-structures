#pragma once
#include <chrono>
#include <iostream>
#include <sstream>

enum class OrderType {
    BUY,
    SELL,
    CANCEL
};

struct Order {
    uint64_t order_id;
    OrderType type;
    double price;
    double quantity;
    u_int64_t timestamp;
    uint64_t sequence_number;

    friend std::ostream& operator<<(std::ostream& os, const Order &o);
};