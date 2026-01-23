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
#include <barrier>

// Order simulation includes
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/random_order_generator.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "order_simulation/market_state.hpp"

// Temporarily defining threads, will pass as parameter
#include "scenarios/test_inputs.hpp"

// custom utils
#include "utils/threads.hpp"
#include "utils/structs.hpp"

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
    const uint64_t TOTAL_THREADS = PRODUCERS + CONSUMERS;
    const uint64_t TOTAL_ORDERS = params.total_orders;
    const uint64_t THREAD_LIMIT = params.thread_order_limit;
    const uint32_t SEED = params.seed;
    constexpr uint32_t PREPROCESS_LIMIT = 10000;

    std::barrier benchmark_barrier(TOTAL_THREADS);

    std::vector<llogs::LatencyStore> thread_latencies(TOTAL_THREADS);
    for (auto &t_l: thread_latencies){
        t_l.enqueue_buffers = std::make_unique<uint64_t>(THREAD_LIMIT);
        t_l.dequeue_buffers = std::make_unique<uint64_t>(THREAD_LIMIT);
    }

    MarketState market_state;

    std::vector<std::thread> producers;
    for (int i = 0; i < PRODUCERS; ++i) {
        int tid = wrapper.addEnqThread();
        producers.emplace_back(
            [&, tid]() {
                RandomOrderGenerator<BenchmarkOrder> gen(market_state, 100 * (tid + 1), SEED + tid);
                
                // perform preprocess here
                // for (uint64_t i = 0; i < PREPROCESS_LIMIT; ++i){
                //     BenchmarkOrder o{};
                //     wrapper.preprocessEnqueue(o, tid);
                // }
                
                // once all threads done, starts actual benchmarking
                benchmark_barrier.arrive_and_wait();

                for (uint64_t count = 0; count < THREAD_LIMIT; ++count){
                    BenchmarkOrder o = gen.generate();
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
                uint64_t *timestamps[THREAD_LIMIT];
                // perform preprocess here
                // for (uint64_t i = 0; i < PREPROCESS_LIMIT; ++i){
                //     wrapper.preprocessDequeue(o, tid);
                // }

                benchmark_barrier.arrive_and_wait();

                for (uint64_t count = 0; count < THREAD_LIMIT; ++count){
                    wrapper.dequeueLatency(o, tid);
                }
            }
        );
    }

    lThread::close(producers, consumers);

    wrapper.processLatencies();
}
