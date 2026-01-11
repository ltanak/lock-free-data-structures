#pragma once

#include <sstream>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include <iostream>

/**
 * Going for a recentering OrderBook, so that it dynamically adapts
 * to price changes like in a real exchange
 */

struct PriceBook {
    static constexpr int NUM_LEVELS = 8192;
    static constexpr int BITMAP_WORDS = (NUM_LEVELS + 63) / 64;

    uint32_t base_price_ticks;    // lowest supported price
    uint32_t centre_price_ticks;  // use market state information

    PriceLevel levels[NUM_LEVELS]; // the number of pricelevels
    uint64_t bitmap[BITMAP_WORDS]; // each bit corresponds to a PriceLevel to see if it is active or not
    uint8_t side; // buy or sell

    auto initialise(uint32_t centre) -> void;
    auto recentre(uint32_t centre) -> void;
    auto shift(uint32_t new_ticks) -> void;
    auto priceToIndex(uint32_t price) -> int;
    auto addOrder(BookOrder*) -> void;
    auto removeOrder(BookOrder*) -> void;
    auto setBit(int index) -> void;
    auto clearBit(int index) -> void;

    auto bestPriceLevel() const -> int;
};