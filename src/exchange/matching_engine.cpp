#include <sstream>
#include <cmath>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "exchange/matching_engine.hpp"

auto MatchingEngine::processOrder(BookOrder* incoming) -> void {
    if (incoming->side == 0){
        matchBuy(incoming);
        if (incoming->quantity > 0) buy_book.addOrder(incoming);
    } else {
        matchSell(incoming);
        if (incoming->quantity > 0) sell_book.addOrder(incoming);
    }
}

auto MatchingEngine::convertOrder(const BenchmarkOrder& incoming) -> BookOrder {
    BookOrder book_order;
    book_order.order_id = incoming.order_id;
    book_order.quantity = incoming.quantity;
    book_order.price_ticks = priceToTicks(incoming.price);
    book_order.side = (incoming.type == OrderType::BUY) ? 0 : 1;
    book_order.next = nullptr;
    book_order.prev = nullptr;
    return book_order;
}

auto MatchingEngine::priceToTicks(double price) -> uint32_t {
    return static_cast<uint32_t>(std::llround(price * price_scale));
}

auto MatchingEngine::matchBuy(BookOrder* order) -> void {
    while (order->quantity > 0){
        int best_sell = sell_book.bestPriceLevel();
        if (best_sell < 0) break;

        uint32_t best_price = sell_book.base_price_ticks + best_sell;
        if (best_price > order->price_ticks) break;

        PriceLevel& level = sell_book.levels[best_sell];
        BookOrder* listStart = level.head;

        uint32_t traded_volume = std::min(order->quantity, listStart->quantity);

        order->quantity -= traded_volume;
        listStart->quantity -= traded_volume;

        if (listStart->quantity == 0){
            sell_book.removeOrder(listStart);
        }
    }
}

auto MatchingEngine::matchSell(BookOrder* order) -> void {
    while (order->quantity > 0){
        int best_buy = buy_book.bestPriceLevel();
        if (best_buy < 0) break;
        
        uint32_t best_price = buy_book.base_price_ticks + best_buy;
        if (best_price < order->price_ticks) break;

        PriceLevel& level = buy_book.levels[best_buy];
        BookOrder* listStart = level.head;
        uint32_t traded_volume = std::min(order->quantity, listStart->quantity);

        order->quantity -= traded_volume;
        listStart->quantity -= traded_volume;

        if (listStart->quantity == 0){
            buy_book.removeOrder(listStart);
        }
    }
}
