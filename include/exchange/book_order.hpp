#pragma once
#include <sstream>

struct BookOrder {
    uint32_t order_id;
    uint32_t tick_index; // book order does not stores prices, stores the index of the "tick" it is in
    uint32_t quantity;
    uint8_t side;

    BookOrder* prev;
    BookOrder* next;
};