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
    // pointers to head and tail of linked list
    BookOrder* head;
    BookOrder* tail;

    // total quantity currently stored
    int total_quantity = 0;

    // O(1) insertion / removal functions
    auto enqueue(BookOrder*) -> void;
    auto dequeue(BookOrder*) -> void;

    // helper methods
    auto isEmpty() -> bool;
    auto clear() -> void;

    // used on book recentering
    auto move(PriceLevel& move_from) -> void;
};
