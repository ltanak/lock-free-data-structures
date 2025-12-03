#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>

// Order simulation includes
#include "order_simulation/order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Temporarily defining threads, will pass as parameter
#include "scenarios/test_inputs.hpp"

/**
 * @brief Stress Test
 * Purpose of test is to benchmark the throughput of multiple producers adding to a data structure
 * Multiple threads will be generating orders and pushing them to the data structure
 * Each producer thread will have its own order generation model
 * Multiple consumer threads will be dequeuing from the data structure at the same time
 * 
 * @note For SPSC data structures, inputs will be 1 producer, 1 consumer thread
 */

void closeThreads(std::vector<std::thread> &producers, std::vector<std::thread> &consumers){
    for (auto& prod: producers){
        prod.join();
    }
    for (auto& cons: consumers){
        cons.join();
    }
}

template <typename DataStructure>
void stressTest(DataStructure &structure, TestParams &params) {
    const uint64_t PRODUCERS = params.thread_count;
    const uint64_t CONSUMERS = params.thread_count;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;

    std::cout << "Running multi producer stress test: " << std::endl;

    std::atomic<bool> running{true};
    std::vector<uint64_t> counts(PRODUCERS, 0);
    MarketState marketState;

    // RandomOrderGenerator<Order> g1 = RandomOrderGenerator<Order>(marketState, 10, 42);
    // RandomOrderGenerator<Order> g2 = RandomOrderGenerator<Order>(marketState, 100, 25);

    // std::vector<std::function<Order()>> gens {
    //     [&]() { return g1.generate() ;},
    //     [&]() { return g2.generate();}
    // };

    // CollectionOrderGenerator<Order> collection(gens, 42);

    std::vector<std::thread> producers;
    for (int i = 0; i < PRODUCERS; ++i) {
        producers.emplace_back(
            [&, i]() {
                uint64_t count = 0;
                RandomOrderGenerator<Order> gen(marketState, 100 * (i + 1), 100 + i);
                while (running.load(std::memory_order_relaxed)){
                    Order o = gen.generate();
                    ++count;
                    structure.enqueue_order(o, i);
                    // will print out result to test it works
                }
                counts[i] = count; 
            }
        );
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        consumers.emplace_back(
            [&, i]() {
                Order o;
                while (running.load(std::memory_order_relaxed)){
                    // structure.dequeue(o, i);
                    // can also print out results here
                }
            }
        );
    }

    std::cout << "Running the threads" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "Timer done" << std::endl;

    running.store(false);
    closeThreads(producers, consumers);

    std::cout << "Counts: ";
    for (auto& c: counts){
        std::cout << c << ", ";
    }

    structure.processLatencies();
    std::cout << std::endl;

    std::cout << "Stress test completed with " << PRODUCERS << " producers and " << CONSUMERS << " consumers.\n";
}
