#pragma once

#include <sstream>
#include "exchange/book_order.hpp"

// double ended queue
struct PriceLevel {
    BookOrder* head;
    BookOrder* tail;
    int total_quantity;

    auto enqueue(BookOrder*) -> void;
    auto dequeue(BookOrder*) -> void;
    bool isEmpty();
};