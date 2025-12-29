#pragma once

#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"

/**
 * Going for a recentering OrderBook, so that it dynamically adapts
 * to price changes like in a real exchange
 */

struct PriceBook {
    static constexpr int NUM_LEVELS = 8192;
    static constexpr int BITMAP_WORDS = (NUM_LEVELS + 63) / 64;

    int32_t base_price_ticks;    // lowest supported price
    int32_t center_price_ticks;  // use market state information

    PriceLevel levels[NUM_LEVELS]; // the number of pricelevels
    uint64_t bitmap[BITMAP_WORDS]; // each bit corresponds to a PriceLevel
    uint8_t side; // buy or sell
};