#include "order_simulation/order.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const Order &order){
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  order.timestamp.time_since_epoch()).count();
    os << "Order {"
       << "id= " << order.order_id
       << ", type= " << (order.type == OrderType::BUY ? "BUY" : "SELL") 
       << ", price= " << order.price
       << ", qty= " << order.quantity
       << ", ts_ms= " << ms
       << ", seq= " << order.sequence_number
       << "}";
    return os;
}