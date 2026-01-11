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
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Temporarily defining threads, will pass as parameter
#include "scenarios/test_inputs.hpp"

// custom utils
#include "utils/threads.hpp"

/**
 * @brief Stress Test
 * Purpose of test is to benchmark the throughput of multiple producers adding to a data structure
 * Multiple threads will be generating orders and pushing them to the data structure
 * Each producer thread will have its own order generation model
 * Multiple consumer threads will be dequeuing from the data structure at the same time
 * 
 * @note For SPSC data structures, inputs will be 1 producer, 1 consumer thread
 */

template <typename Wrapper>
void stressTest(Wrapper &wrapper, TestParams &params) {
    const uint64_t PRODUCERS = params.thread_count;
    const uint64_t CONSUMERS = params.thread_count;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;
    const uint32_t SEED = params.seed;

    MarketState market_state;

    std::vector<std::thread> producers;
    for (int i = 0; i < PRODUCERS; ++i) {
        int tid = wrapper.addEnqThread();
        producers.emplace_back(
            [&, tid]() {
                uint64_t count = 0;
                RandomOrderGenerator<BenchmarkOrder> gen(market_state, 100 * (tid + 1), SEED + tid);
                while (true){
                    if (count >= THREAD_LIMIT) break;

                    BenchmarkOrder o = gen.generate();
                    ++count;
                    
                    wrapper.enqueueOrder(o, tid);
                }
            }
        );
    }

    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDeqThread();
        consumers.emplace_back(
            [&, tid]() {
                BenchmarkOrder o;
                uint64_t count = 0;
                while (true) {
                    if (count >= THREAD_LIMIT) break;
                    ++count;

                    wrapper.dequeueLatency(o, tid);
                }
            }
        );
    }

    lThread::close(producers, consumers);

    wrapper.processLatencies();
}
