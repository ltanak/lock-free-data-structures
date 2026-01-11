#pragma once
#include <sstream>
#include <iostream>
#include <stdint.h>

/**
 * BookOrder struct represents the order used within the OrderBook
 * Differs from BenchmarkOrder
 * Stores prices as ticks - integer representation of a float value
 * Each order points to adjacent orders at same PriceLevel, added in FIFO sequence
   to maintain price-time priority
 */

struct BookOrder {
    uint32_t order_id;
    uint32_t price_ticks;
    uint32_t quantity;
    uint8_t side;

    // pointers to adjacent orders
    BookOrder* prev;
    BookOrder* next;
};
