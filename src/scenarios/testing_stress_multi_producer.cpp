#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

void multiProducerStressTest() {
    std::cout << "begin?" << std::endl;
    /**
        - Each thread has its own order generation model
        - Each thread pushes directly to the data structure
        - Consumer threads will deque at the same time
        - Don't require the market state thread to be running for this test (for now)
     */
    constexpr int numProducers = 4;
    constexpr int numConsumers = 4;
    std::atomic<bool> running{true};
    std::vector<uint64_t> counts(numProducers, 0);

    MarketState marketState;

    std::vector<std::thread> producers;
    for (int i = 0; i < numProducers; ++i) {
        producers.emplace_back(
            [&, i]() {
                uint64_t count = 0;
                RandomOrderGenerator<Order> gen(marketState, 100 * (i + 1), 100 + i);
                while (running.load(std::memory_order_relaxed)){
                    Order o = gen.generate();
                    ++count;
                    // datastructure.enqueue(o);
                    // will print out result to test it works
                    
                    // std::cout << "ID: " << o.order_id << ", Type: " << (o.type == OrderType::BUY ? "BUY" : "SELL") 
                    // << ", Price: " << o.price << ", Quantity: " << o.quantity << std::endl;
                }
                counts[i] = count; 
            }
        );
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < numConsumers; ++i) {
        consumers.emplace_back(
            [&, i]() {
                Order o;
                while (running.load(std::memory_order_relaxed)){
                    // o = queue.deque();
                    // can also print out results here
                }
            }
        );
    }

    std::cout << "Running the threads" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Timer done" << std::endl;

    running.store(false);
    for (auto& prod: producers){
        prod.join();
    }

    for (auto& cons: consumers){
        cons.join();
    }

    std::cout << "Counts: ";
    for (auto& c: counts){
        std::cout << c << ", ";
    }
    std::cout << std::endl;

    std::cout << "Stress test completed with " << numProducers << " producers and " << numConsumers << " consumers.\n";
}