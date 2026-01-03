#pragma once

#include <sstream>
#include <iostream>
#include "exchange/book_order.hpp"

// double ended queue
struct alignas(64) PriceLevel {
    BookOrder* head;
    BookOrder* tail;
    int total_quantity = 0;

    auto enqueue(BookOrder*) -> void;
    auto dequeue(BookOrder*) -> void;
    auto isEmpty() -> bool;
    auto clear() -> void;
    auto move(PriceLevel& move_from) -> void;
};