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
#include "utils/timing.hpp"

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

    std::cout << "testing 1" << std::endl;

    std::vector<llogs::LatencyStore> thread_latencies(TOTAL_THREADS);
    for (auto &t_l: thread_latencies){
        t_l.enqueue_buffers = std::make_unique<uint64_t[]>(THREAD_LIMIT);
        t_l.dequeue_buffers = std::make_unique<uint64_t[]>(THREAD_LIMIT);
    }


    MarketState market_state;

    std::vector<std::thread> producers;
    for (int i = 0; i < PRODUCERS; ++i) {
        int tid = wrapper.addEnqThread();
        producers.emplace_back(
            [&, tid, index = i, cpu = tid]() {
                RandomOrderGenerator<BenchmarkOrder> gen(market_state, 100 * (tid + 1), SEED + tid);
                lThread::pin_thread(cpu);
                auto *local_buffer = thread_latencies[index].enqueue_buffers.get();


                // perform preprocess here
                // for (uint64_t i = 0; i < PREPROCESS_LIMIT; ++i){
                //     BenchmarkOrder o{};
                //     wrapper.preprocessEnqueue(o, tid);
                // }
                
                // once all threads done, starts actual benchmarking
                benchmark_barrier.arrive_and_wait();

                for (uint64_t i = 0; i < THREAD_LIMIT; ++i){
                    BenchmarkOrder o = gen.generate();

                    // start timestamp
                    uint64_t t0 = ltime::rdtsc_lfence();
                    // enqueue datastructure
                    wrapper.enqueueOrder(o, tid);
                    uint64_t t1 = ltime::rdtsc_lfence();
                    local_buffer[i] = t1 - t0;
                }

            }
        );
    }

    std::cout << "here 2" << std::endl;

    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; ++i) {
        int tid = wrapper.addDeqThread();
        consumers.emplace_back(
            [&, tid, index = i, cpu = tid]() {
                BenchmarkOrder o;
                lThread::pin_thread(cpu);

                auto *local_buffer = thread_latencies[index].dequeue_buffers.get();
                
                // perform preprocess here
                // for (uint64_t i = 0; i < PREPROCESS_LIMIT; ++i){
                //     wrapper.preprocessDequeue(o, tid);
                // }

                benchmark_barrier.arrive_and_wait();

                for (uint64_t count = 0; count < THREAD_LIMIT; ++count){

                    // start timestamp
                    uint64_t t0 = ltime::rdtsc_lfence();
                    wrapper.dequeueLatency(o, tid);
                    uint64_t t1 = ltime::rdtsc_lfence();
                    local_buffer[count] = t1 - t0;
                }
            }
        );
    }

    std::cout << "here 3" << std::endl;

    lThread::close(producers, consumers);

    std::cout << "here 4" << std::endl;

    std::vector<uint64_t> local_enqueues;
    std::vector<uint64_t> local_dequeues;
    local_enqueues.reserve(PRODUCERS * THREAD_LIMIT);
    local_dequeues.reserve(CONSUMERS * THREAD_LIMIT);

    for (auto &t_l: thread_latencies){
        std::copy(t_l.enqueue_buffers.get(), t_l.enqueue_buffers.get() + THREAD_LIMIT, std::back_inserter(local_enqueues));
        std::copy(t_l.dequeue_buffers.get(), t_l.dequeue_buffers.get() + THREAD_LIMIT, std::back_inserter(local_dequeues));
    }

    wrapper.setLatencyVectors(local_enqueues, local_dequeues);
    // wrapper.latencies_dequeue_ = std::move(local_dequeues);

    wrapper.processLatencies();
}
