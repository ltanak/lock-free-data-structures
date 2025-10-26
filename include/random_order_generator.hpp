#include "order_generator.hpp"
#include <random>
#include <chrono>

template <typename TOrder>
class RandomOrderGenerator : public OrderGenInterface<TOrder, RandomOrderGenerator<TOrder>> {
    
    // will add to this
};