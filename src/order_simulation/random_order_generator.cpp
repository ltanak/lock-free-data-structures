#include "order.hpp"
#include "random_order_generator.hpp"
#include <random>
#include <chrono>
#include <optional>

template<typename TOrder> // as constructor no return type, but normally for other functions you put void, int, etc
RandomOrderGenerator<TOrder>::RandomOrderGenerator(double minPrice, double maxPrice, double maxQuantity, std::optional<uint32_t> seed){
    minPrice_ = minPrice;
    maxPrice_ = maxPrice;
    maxQuantity_ = maxQuantity;
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
    priceDist_ = std::uniform_real_distribution<double>(minPrice_, maxPrice_);
    quantityDist_ = std::uniform_real_distribution<double>(1.0, maxQuantity_);
    sideDist_ = std::uniform_int_distribution<int>(0, 1);
}

template<typename TOrder>
TOrder RandomOrderGenerator<TOrder>::generateOrder(){
    orderId_++;
    OrderType orderType = (sideDist_(rng_) == 0) ? OrderType::BUY : OrderType::SELL;
    double price = priceDist_(rng_);
    double quantity = quantityDist_(rng_);
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp = std::chrono::high_resolution_clock::now();
    return TOrder{orderId_, orderType, price, quantity, timestamp};
}

template class RandomOrderGenerator<Order>;
