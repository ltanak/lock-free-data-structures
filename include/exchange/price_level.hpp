#pragma once

#include <sstream>
#include <iostream>
#include <stdint.h>

#include "exchange/book_order.hpp"

/**
 * PriceLevel struct
 * Stores all the orders as a FIFO linkedlist that match its pricelevel
 */
struct alignas(64) PriceLevel {
    BookOrder* head;
    BookOrder* tail;
    int total_quantity = 0;

    auto enqueue(BookOrder*) -> void;
    auto dequeue(BookOrder*) -> void;
    auto isEmpty() -> bool;
    auto clear() -> void;

    // used on book recentering
    auto move(PriceLevel& move_from) -> void;
};