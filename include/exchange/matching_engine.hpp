#pragma once

#include <sstream>
#include <string>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "exchange/trades_cycle.hpp"

template<typename TOrder>
struct MatchingEngine {
    PriceBook buy_book;
    PriceBook sell_book;

    double tick_size = 0.01; // for prices such as 123.45
    uint32_t price_scale = 100;

    int cycle = 0;

    // constructor for matchingengine
    explicit MatchingEngine(double);

    auto processOrder(BookOrder*) -> TradesCycle;
    auto convertOrder(const TOrder&) -> BookOrder;
    auto priceToTicks(double) -> uint32_t;
    auto ticksToPrice(uint32_t) -> double;
    auto matchBuy(BookOrder*) -> TradesCycle;
    auto matchSell(BookOrder*) -> TradesCycle;
};
