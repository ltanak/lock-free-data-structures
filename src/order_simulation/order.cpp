#include "order_simulation/order.hpp"
#include "utils/timing.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const Order &order){
    uint64_t clocks = lTime::rdtscp_inline();
    os << "Order {"
       << "id= " << order.order_id
       << ", type= " << (order.type == OrderType::BUY ? "BUY" : "SELL") 
       << ", price= " << order.price
       << ", qty= " << order.quantity
       << ", ts_clocks= " << clocks
       << ", seq= " << order.sequence_number
       << "}";
    return os;
}