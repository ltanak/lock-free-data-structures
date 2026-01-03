#pragma once

#include <sstream>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "order_simulation/benchmark_order.hpp"

struct MatchingEngine {
    PriceBook buy_book;
    PriceBook sell_book;

    uint32_t tick_size = 0.01; // for prices such as 123.45
    uint32_t price_scale;

    auto processOrder(BenchmarkOrder*) -> void;
    auto convertOrder(const BenchmarkOrder&) -> BookOrder;
    auto priceToTicks(double) -> uint32_t;
    auto matchBuy() -> void;
    auto matchSell() -> void;

};