#pragma once

#include <sstream>
#include <string>

#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "exchange/trades_cycle.hpp"
#include "order_simulation/benchmark_order.hpp"

/**
 * MatchingEngine struct
 * Entry point for the exchange, stores books and initialises ticks for 'traded-commodity'
 */

template<typename TOrder>
struct MatchingEngine {

    // buy and sell books
    PriceBook buy_book;
    PriceBook sell_book;

    double tick_size = 0.01; // for prices such as 123.45
    uint32_t price_scale = 100;

    // cycle to count which iteration we are on in the exchange matching
    int cycle = 0;

    // constructor for matchingengine
    explicit MatchingEngine(double);

    auto processOrder(BookOrder*) -> TradesCycle;
    auto convertOrder(const TOrder&) -> BookOrder;

    // conversion functions
    auto priceToTicks(double) -> uint32_t;
    auto ticksToPrice(uint32_t) -> double;

    // use corresponding function for incoming order
    auto matchBuy(BookOrder*) -> TradesCycle;
    auto matchSell(BookOrder*) -> TradesCycle;
};
