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

static void closeThreads2(std::vector<std::thread> &producers, std::vector<std::thread> &consumers){
    for (auto& prod: producers){
        prod.join();
    }
    for (auto& cons: consumers){
        cons.join();
    }
}

template <typename Wrapper>
void stressTest(Wrapper &wrapper, TestParams &params) {
    const uint64_t PRODUCERS = params.thread_count;
    const uint64_t CONSUMERS = params.thread_count;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;

    std::cout << "Running multi producer stress test: " << std::endl;

    // std::atomic<uint64_t> completed{0};
    // std::vector<uint64_t> counts(PRODUCERS, 0);
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
        int tid = wrapper.addThread();
        producers.emplace_back(
            [&, i]() {
                uint64_t count = 0;
                RandomOrderGenerator<Order> gen(marketState, 100 * (i + 1), 100 + i);
                while (true){
                    if (count >= THREAD_LIMIT) break;

                    Order o = gen.generate();
                    ++count;
                    
                    wrapper.enqueue_order(o, tid);
                }
                // counts[i] = count; 
            }
        );
    }

    // NEED TO ADD CODE FOR CONSUMERS

    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDequeueThread();
        consumers.emplace_back(
            [&, i]() {
                // Order o;
                // while (running.load(std::memory_order_relaxed)){
                //     // wrapper.dequeue(o, i);
                //     // can also print out results here
                // }
                Order o;
                uint64_t count = 0;
                while (true) {
                    if (count >= THREAD_LIMIT) break;
                    ++count;

                    wrapper.dequeue_order(o, tid);
                }


                // uint64_t count = 0;
                // RandomOrderGenerator<Order> gen(marketState, 100 * (i + 1), 100 + i);
                // while (true){
                //     if (count >= THREAD_LIMIT) break;

                //     Order o = gen.generate();
                //     ++count;
                    
                //     wrapper.enqueue_order(o, tid);
                // }
                // counts[i] = count; 
            }
        );
    }

    std::cout << "Running the threads" << std::endl;

    closeThreads2(producers, consumers);

    // std::cout << "Counts: ";
    // for (auto& c: counts){
    //     std::cout << c << ", ";
    // }

    wrapper.processLatencies();
    std::cout << std::endl;

    std::cout << "Stress test completed with " << PRODUCERS << " producers and " << CONSUMERS << " consumers.\n";
}
