#pragma once
#include "order.hpp"
#include <random>


/**
 * @brief Interface for generating orders
 * @tparam TOrder The type of order to generate
 * @return TOrder The generated order
 * Using CRTP instead of virtual functions for performance reasons
 * Every derived class implements its own generateOrder method
 */
template <typename TOrder, typename OrderGenImpl>
class OrderGenInterface {
    public:
        TOrder generate() {
            return static_cast<OrderGenImpl*>(this)->generateOrder();
        }
};