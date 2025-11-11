#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <queue>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"
#include "scenarios/testing_order_single_consumer.hpp"

void singleConsumerOrderTest() {
    // create a vector filled with a bunch of orders & market updates
    // NEED TO PARAMETERISE THE FUNCTIONS SO WE CAN RUN DIFFERENT AMOUNT OF ORDERS
    // apply an ordering value to each of these (using sequence_number)

    // then we will enqueue all of these in order into the data structure

    // then we will deuqueue all of these in order
    // we will only benchmark from the dequeing point

    std::queue<Order> ordersQueue;
    MarketState marketState;
    RandomOrderGenerator<Order> g1 = RandomOrderGenerator<Order>(marketState, 10, 42);
    RandomOrderGenerator<Order> g2 = RandomOrderGenerator<Order>(marketState, 100, 25);

    std::vector<std::function<Order()>> gens {
        [&]() { return g1.generate() ;},
        [&]() { return g2.generate();}
    };

    CollectionOrderGenerator<Order> collection(gens, 42);
    for (int i = 0; i < 20; ++i){
        Order o = collection.generate();
        o.sequence_number = i;
        ordersQueue.emplace(o); // emplace copies the actual thing, rather than doing it by pointer
    }
    std::cout << "Orders now going to be popped -> enqueued" << std::endl;


    // enqueueing onto the data structure
    while (!ordersQueue.empty()){
        Order o1 = ordersQueue.front(); // CHECK THIS - NEED TO MAKE SURE IT ACTUALLY 
        ordersQueue.pop();
        std::cout << "ID: " << o1.order_id << ", Type: " << (o1.type == OrderType::BUY ? "BUY" : "SELL") 
        << ", Price: " << o1.price << ", Quantity: " << o1.quantity << ", Sequence: " << o1.sequence_number << std::endl;

        // datastructure.enqueue(o1);
    }


    // dequeing from the data structure

    /*
    while (!datastructure.empty()){
        Order o2 = datastructure.dequeue();
        // output results or measure results here
    }
    
    */


}