#pragma once
#include <sstream>
#include <iostream>
#include <stdint.h>


struct BookOrder {
    uint32_t order_id;
    // book order does not stores prices, 
    // stores the index of the "tick" it is in
    uint32_t price_ticks;
    uint32_t quantity;
    uint8_t side;

    BookOrder* prev;
    BookOrder* next;
};