#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/market_state.hpp"
#include <random>
#include <chrono>
#include <optional>

template<typename TOrder>
RandomOrderGenerator<TOrder>::RandomOrderGenerator(MarketState &market, double maxQuantity, std::optional<uint32_t> seed){
    maxQuantity_ = maxQuantity;
    marketState_ = &market;
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
    quantityDist_ = std::uniform_real_distribution<double>(1.0, maxQuantity_);
    sideDist_ = std::uniform_int_distribution<int>(0, 1);
}

template<typename TOrder>
TOrder RandomOrderGenerator<TOrder>::generateOrder(){
    orderId_++;

    double base = marketState_->getPrice();
    double vol = marketState_->getVolatility();
    double bias = marketState_->getBias();

    // gaussian distr around base price, scaled by volatility
    std::normal_distribution<double> priceDist(base, base * vol);
    double price = priceDist(rng_);

    // Adjust side probability based on trend bias
    bool isBuy = (sideDist_(rng_) < (0.5 + bias * 0.5));
    OrderType type = isBuy ? OrderType::BUY : OrderType::SELL;

    double quantity = quantityDist_(rng_);
    auto timestamp = std::chrono::high_resolution_clock::now();

    return TOrder{orderId_, type, price, quantity, timestamp};
}

template class RandomOrderGenerator<Order>;
