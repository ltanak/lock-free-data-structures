#pragma once

#include <sstream>
#include <string>

#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "exchange/trades_cycle.hpp"
#include "order_simulation/benchmark_order.hpp"

/**
 * @class MatchingEngine
 * @brief Core financial exchange matching engine implementing price-time priority.
 *
 * implemented a limit order book (LOB) matching engine using two separate price books
 * (buy and sell sides) with dynamic recentering
 *
 * @tparam TOrder User-defined order type to be matched
 */
template<typename TOrder>
struct MatchingEngine {

    // buy and sell books
    PriceBook buy_book;
    PriceBook sell_book;

    // ticks / pricing
    double tick_size = 0.01; // for prices such as 123.45
    uint32_t price_scale = 100;

    // cycle to count which iteration we are on in the exchange matching
    int cycle = 0;

    // constructor for matchingengine
    explicit MatchingEngine(double);

    // order processing functions
    auto processOrder(BookOrder*) -> TradesCycle;
    auto convertOrder(const TOrder&) -> BookOrder;

    // price conversion functions
    auto priceToTicks(double) -> uint32_t;
    auto ticksToPrice(uint32_t) -> double;

    // use corresponding function for incoming order
    auto matchBuy(BookOrder*) -> TradesCycle;
    auto matchSell(BookOrder*) -> TradesCycle;
};
