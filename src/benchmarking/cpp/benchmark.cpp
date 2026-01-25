#include <random>
#include <optional>
#include <atomic>
#include <fstream>
#include <sstream>

#include "benchmarking/benchmark.hpp"
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "exchange/trades_cycle.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/collection_order_generator.hpp"
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"
#include "utils/files.hpp"

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::BenchmarkWrapper(DataStructure &structure, TestParams &params)
: structure_(structure), TOTAL_ORDERS_(params.total_orders), NUM_THREADS_(params.thread_count), THREAD_LIMIT_(params.thread_order_limit), exchange_(100)
{
    if (NUM_THREADS_ == 0 || THREAD_LIMIT_ == 0 || TOTAL_ORDERS_ == 0) {
        std::cerr << "Invalid params (zero)" << std::endl;
        std::abort();
    }
}

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::~BenchmarkWrapper(){
    return;
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addEnqThread(){
    return enqueue_thread_id_.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addDeqThread(){
    return dequeue_thread_id.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::preprocessEnqueue(TOrder &o, int threadId) {
    // pre-process data structure
    bool enqueued = structure_.enqueue(o);
    return enqueued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::preprocessDequeue(TOrder &o, int threadId) {
    bool dequeued = structure_.dequeue(o);
    return dequeued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::enqueueOrder(TOrder &o, int threadId) {
    bool enqueued = structure_.enqueue(o);
    return enqueued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeueLatency(TOrder &o, int threadId) {
    bool dequeued = structure_.dequeue(o);
    return dequeued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeueOrdering(TOrder &o, int threadId) {
    // maybe add logic for if transaction wasn't successful
    bool dequeued = structure_.dequeue(o);
    return dequeued;
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::setLatencyVectors(const std::vector<uint64_t> &enqueue, const std::vector<uint64_t> &dequeue){
    latencies_enqueue_ = enqueue;
    latencies_dequeue_ = dequeue;
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::setOrderingVectors(const std::vector<uint64_t> &timestamps, const std::vector<uint64_t> &sequence){
    timestamps_dequeue_ = timestamps;
    sequence_dequeue_ = sequence;
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processLatencies(){
    double cycles_per_ns = ltime::measure_tsc_ghz();
    std::vector<double> e_ns_latencies;
    std::vector<double> d_ns_latencies;
    double sumEnq = 0;
    double sumDeq = 0;

    for (size_t i = 0; i < TOTAL_ORDERS_; ++i) {
        // convert rdtscp into nano seconds
        double e_ns = latencies_enqueue_[i] / cycles_per_ns;
        sumEnq += e_ns;
        e_ns_latencies.push_back(e_ns);

        double d_ns = latencies_dequeue_[i] / cycles_per_ns;
        sumDeq += d_ns;
        d_ns_latencies.push_back(d_ns);
    }
    sumEnq /= TOTAL_ORDERS_;
    sumDeq /= TOTAL_ORDERS_;

    // std out for average latencies (will be later included in report generation)
    std::cout << "Avg Enqueue Latency (ns): " << sumEnq << std::endl;
    std::cout << "Avg Dequeue Latency (ns): " << sumDeq << std::endl;
    latencies::write(e_ns_latencies, d_ns_latencies);
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processOrders(CollectionOrderGenerator<BenchmarkOrder> &generator){
    std::vector<uint64_t> actual_order;
    std::vector<uint64_t> expected_order;
    std::vector<TOrder> orders;

    std::vector<std::pair<uint64_t, uint64_t>> t_s; // timestamp, then sequence
    t_s.reserve(TOTAL_ORDERS_);

    // store timestamp and sequence number from dequeue
    for (size_t i = 0; i < TOTAL_ORDERS_; ++i) {
        t_s.emplace_back(timestamps_dequeue_[i], sequence_dequeue_[i]); // make pair of each element, then sort by timestamp
    }

    // sort based on timestamp
    std::sort(t_s.begin(), t_s.end(),
        [](const auto &a, const auto &b) {
            return a.first < b.first;
        }
    );

    // create vector of final sequence
    for (size_t i = 0; i < TOTAL_ORDERS_; ++i){
        actual_order.push_back(t_s[i].second);
    }

    // use seeded generator to get expected sequence of IDs
    for (size_t i = 0; i < TOTAL_ORDERS_; ++i){
        TOrder order = generator.generate();
        orders.push_back(order);
        expected_order.push_back(order.order_id);
    }
    ordering::write(expected_order, actual_order);

    processMatching(orders, actual_order);
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processMatching(std::vector<TOrder>& orders, std::vector<uint64_t>& actual_order) {
    // Convert all orders to BookOrder and store them persistently
    std::vector<BookOrder> book_orders;
    std::vector<uint32_t> original_quantities;
    book_orders.reserve(orders.size());
    original_quantities.reserve(orders.size());
    
    // convert the TOrder / BenchmarkOrder struct into a BookOrder struct
    for (const TOrder& order: orders){
        BookOrder converted_order = exchange_.convertOrder(order);

        // store original quantities (to reset values for second pass)
        original_quantities.push_back(converted_order.quantity);
        book_orders.push_back(converted_order);
    }

    // Process orders in their original (expected) order
    std::vector<TradesCycle> expected_cycles;
    expected_cycles.reserve(book_orders.size());

    // takes original order sequence and puts them through exchange
    for (BookOrder& b_order: book_orders){
        TradesCycle trade = exchange_.processOrder(&b_order);

        // add each tradecycle (which stoes the cycle count, and all the matched prices and quantities)
        expected_cycles.push_back(trade);
    }

    // Restore original quantities before second pass
    for (size_t i = 0; i < book_orders.size(); ++i) {
        book_orders[i].quantity = original_quantities[i];
        book_orders[i].next = nullptr;  // reset pointers
        book_orders[i].prev = nullptr;
    }

    // mapping of order_id to its position in actual_order
    std::unordered_map<uint64_t, size_t> order_pos;
    for (size_t i = 0; i < actual_order.size(); ++i){
        order_pos[actual_order[i]] = i;
    }

    // sort based on their position in actual_order (their sequence on the output from processOrders)
    std::sort(book_orders.begin(), book_orders.end(),
        [&order_pos](const BookOrder& a, const BookOrder& b){
            auto pos_a = order_pos.find(a.order_id);
            auto pos_b = order_pos.find(b.order_id);

            if (pos_a == order_pos.end()) return false;
            if (pos_b == order_pos.end()) return true;
            
            return pos_a->second < pos_b->second;
        }
    );

    // process sorted orders through cleared exchange
    std::vector<TradesCycle> actual_cycles;
    actual_cycles.reserve(book_orders.size());

    // chekc this - unsafe / memory overhead
    MatchingEngine<TOrder> new_exchange(100.00);
    exchange_ = new_exchange;

    for (BookOrder& b_order: book_orders){
        // go through exchange with new ordering
        TradesCycle trade = exchange_.processOrder(&b_order);
        actual_cycles.push_back(trade);
    }

    // write results to csv
    exchange::write(expected_cycles, actual_cycles);
}

template class BenchmarkWrapper<RegularQueue<BenchmarkOrder>, BenchmarkOrder>;
template class BenchmarkWrapper<MCLockFreeQueue<BenchmarkOrder>, BenchmarkOrder>;
template class BenchmarkWrapper<MCConcurrentQueue<BenchmarkOrder>, BenchmarkOrder>;
