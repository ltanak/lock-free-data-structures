#include <sstream>
#include <cmath>
#include <vector>
#include "exchange/book_order.hpp"
#include "exchange/price_book.hpp"
#include "exchange/price_level.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "exchange/matching_engine.hpp"

#include "utils/files.hpp"
#include "exchange/trades_cycle.hpp"

template<typename TOrder>
MatchingEngine<TOrder>::MatchingEngine(double centre_price): tick_size(0.01), price_scale(100) {
    uint32_t centre_ticks = static_cast<uint32_t>(std::llround(centre_price * price_scale));

    buy_book.initialise(centre_ticks);
    buy_book.side = 0;   // BUY order

    sell_book.initialise(centre_ticks);
    sell_book.side = 1;  // SELL order
}

template<typename TOrder>
auto MatchingEngine<TOrder>::processOrder(BookOrder* incoming) -> TradesCycle {
    std::cout << "ProcessOrder: side=" << (int)incoming->side << ", qty=" << incoming->quantity << ", price_ticks=" << incoming->price_ticks << "\n";
    TradesCycle tc;
    if (incoming->side == 0){
        tc = matchBuy(incoming);
        if (incoming->quantity > 0) buy_book.addOrder(incoming);
    } else {
        tc = matchSell(incoming);
        if (incoming->quantity > 0) sell_book.addOrder(incoming);
    }
    cycle++;
    return tc;
}

template<typename TOrder>
auto MatchingEngine<TOrder>::convertOrder(const TOrder& incoming) -> BookOrder {
    BookOrder book_order;
    book_order.order_id = incoming.order_id;
    book_order.quantity = incoming.quantity;
    book_order.price_ticks = priceToTicks(incoming.price);

    // CHECK THIS -> THIS MIGHT CAUSE SOME ISSUES
    book_order.side = (incoming.type == OrderType::BUY) ? 0 : 1;
    book_order.next = nullptr;
    book_order.prev = nullptr;
    return book_order;
}

template<typename TOrder>
auto MatchingEngine<TOrder>::priceToTicks(double price) -> uint32_t {
    return static_cast<uint32_t>(std::llround(price * price_scale));
}

template<typename TOrder>
auto MatchingEngine<TOrder>::ticksToPrice(uint32_t ticks) -> double {
    return static_cast<double>(ticks) / 100.00;
}

template<typename TOrder>
auto MatchingEngine<TOrder>::matchBuy(BookOrder* order) -> TradesCycle {
    std::vector<double> prices;
    std::vector<uint32_t> quantities;
    while (order->quantity > 0){
        int best_sell = sell_book.bestPriceLevel();
        if (best_sell < 0) break;

        uint32_t best_price = sell_book.base_price_ticks + best_sell;
        if (best_price > order->price_ticks) break;

        PriceLevel& level = sell_book.levels[best_sell];
        BookOrder* listStart = level.head;

        if (listStart == nullptr) {
            std::cerr << "ERROR: listStart is nullptr but level exists!\n";
            break;
        }

        uint32_t traded_volume = std::min(order->quantity, listStart->quantity);
        
        if (traded_volume == 0) {
            std::cerr << "ERROR: traded_volume is 0. order->quantity=" << order->quantity 
                      << " listStart->quantity=" << listStart->quantity << "\n";
            break;
        }

        double real_price = ticksToPrice(best_price);

        prices.push_back(real_price);
        quantities.push_back(traded_volume);

        order->quantity -= traded_volume;
        listStart->quantity -= traded_volume;
        level.total_quantity -= traded_volume;

        if (listStart->quantity == 0){
            sell_book.removeOrder(listStart);
        }
    }
    return TradesCycle{cycle, prices, quantities};
}

template<typename TOrder>
auto MatchingEngine<TOrder>::matchSell(BookOrder* order) -> TradesCycle {
    std::vector<double> prices;
    std::vector<uint32_t> quantities;
    while (order->quantity > 0){
        int best_buy = buy_book.bestPriceLevel();
        if (best_buy < 0) break;
        
        uint32_t best_price = buy_book.base_price_ticks + best_buy;
        if (best_price < order->price_ticks) break;

        PriceLevel& level = buy_book.levels[best_buy];
        BookOrder* listStart = level.head;
        uint32_t traded_volume = std::min(order->quantity, listStart->quantity);

        double real_price = ticksToPrice(best_price);

        prices.push_back(real_price);
        quantities.push_back(traded_volume);

        order->quantity -= traded_volume;
        listStart->quantity -= traded_volume;
        level.total_quantity -= traded_volume;

        if (listStart->quantity == 0){
            buy_book.removeOrder(listStart);
        }
    }
    return TradesCycle{cycle, prices, quantities};
}

template struct MatchingEngine<BenchmarkOrder>;
