#pragma once

#include <sstream>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "order_simulation/benchmark_order.hpp"

struct MatchingEngine {
    PriceBook buy_book;
    PriceBook sell_book;

    double tick_size = 0.01; // for prices such as 123.45
    uint32_t price_scale = 100;

    auto processOrder(BookOrder*) -> void;
    auto convertOrder(const BenchmarkOrder&) -> BookOrder;
    auto priceToTicks(double) -> uint32_t;
    auto matchBuy(BookOrder*) -> void;
    auto matchSell(BookOrder*) -> void;

};