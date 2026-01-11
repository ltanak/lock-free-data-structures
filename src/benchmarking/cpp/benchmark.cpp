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
: structure_(structure), local_index_enq_(params.thread_count, 0), local_index_deq_(params.thread_count, 0), TOTAL_ORDERS_(params.total_orders), NUM_THREADS_(params.thread_count), THREAD_LIMIT_(params.thread_order_limit), exchange_(100)
{
    latencies_enqueue_ = new uint64_t[params.total_orders];
    latencies_dequeue_ = new uint64_t[params.total_orders];
    sequence_dequeue_ = new uint64_t[params.total_orders];
    timestamps_dequeue_ = new uint64_t[params.total_orders];
    if (NUM_THREADS_ == 0 || THREAD_LIMIT_ == 0 || TOTAL_ORDERS_ == 0) {
        std::cerr << "Invalid params (zero)" << std::endl;
        std::abort();
    }
}

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::~BenchmarkWrapper(){
    delete[] latencies_enqueue_;
    delete[] latencies_dequeue_;
    delete[] sequence_dequeue_;
    delete[] timestamps_dequeue_;
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
bool BenchmarkWrapper<DataStructure, TOrder>::enqueueOrder(TOrder &o, int threadId) {
    // start timer
    uint64_t t0 = lTime::rdtscp_inline();
    // enqueue datastructure
    bool enqueued = structure_.enqueue(o);
    // end timer
    uint64_t t1 = lTime::rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + local_index_enq_[threadId]++; // per-producer index
    latencies_enqueue_[idx] = delta;
    return enqueued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeueLatency(TOrder &o, int threadId) {
    uint64_t t0 = lTime::rdtscp_inline();
    // dequeue datastructure
    bool dequeued = structure_.dequeue(o);
    // end timer
    uint64_t t1 = lTime::rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + local_index_deq_[threadId]++; // per-consumer index
    latencies_dequeue_[idx] = delta;
    return dequeued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeueOrdering(TOrder &o, int threadId) {
    bool dequeued = structure_.dequeue(o);
    uint64_t idx = threadId * THREAD_LIMIT_ + local_index_deq_[threadId]++;

    sequence_dequeue_[idx] = o.sequence_number;
    timestamps_dequeue_[idx] = lTime::rdtscp_inline();

    // maybe add logic for if transaction wasn't successful
    return dequeued;
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processLatencies(){
    double cycles_per_ns = lTime::measure_tsc_ghz();
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
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    // create vector of final sequence
    for (size_t i = 0; i < TOTAL_ORDERS_; ++i){
        actual_order.push_back(t_s[i].second);
    }

    // use seeded generator to get expected sequence of IDs
    for (size_t i = 0; i < TOTAL_ORDERS_; ++i){
        // std::cout << "Order sequence: " << sequene_dequeue_[i] << std::endl;
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
