#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <order_generator.hpp>
#include <random_order_generator.hpp>

int main() {
    std::cout << "running code" << std::endl;
    RandomOrderGenerator<Order> orderGen = RandomOrderGenerator<Order>(10.0, 100.0, 50.0, 24);
    Order o = orderGen.generate();
    std::cout << "ID: " << o.order_id << ", Type: " << (o.type == OrderType::BUY ? "BUY" : "SELL") 
              << ", Price: " << o.price << ", Quantity: " << o.quantity << std::endl;
    return 0;
}