#include "order_simulation/benchmark_order.hpp"
#include "utils/timing.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

std::ostream& operator<<(std::ostream& os, const BenchmarkOrder &order){
    uint64_t clocks = ltime::rdtsc_lfence();
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
