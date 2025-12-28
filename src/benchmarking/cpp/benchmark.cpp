#include <random>
#include <optional>
#include <atomic>

#include "order_simulation/benchmark_order.hpp"
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "scenarios/test_inputs.hpp"
#include "utils/timing.hpp"

#include "benchmarking/benchmark.hpp"

#include "order_simulation/collection_order_generator.hpp"

#include "utils/files.hpp"

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::BenchmarkWrapper(DataStructure &structure, TestParams &params)
: structure_(structure), localIndexEnq_(params.thread_count, 0), localIndexDeq_(params.thread_count, 0), TOTAL_ORDERS_(params.total_orders), NUM_THREADS_(params.thread_count), THREAD_LIMIT_(params.thread_order_limit)
{
    latencies_enqueue = new uint64_t[params.total_orders];
    latencies_dequeue = new uint64_t[params.total_orders];
    sequence_dequeue = new uint64_t[params.total_orders];
    timestamps_dequeue = new uint64_t[params.total_orders];
    if (NUM_THREADS_ == 0 || THREAD_LIMIT_ == 0 || TOTAL_ORDERS_ == 0) {
        std::cerr << "Invalid params (zero)" << std::endl;
        std::abort();
    }
}

template<typename DataStructure, typename TOrder>
BenchmarkWrapper<DataStructure, TOrder>::~BenchmarkWrapper(){
    delete[] latencies_enqueue;
    delete[] latencies_dequeue;
    delete[] sequence_dequeue;
    delete[] timestamps_dequeue;
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addThread(){
    return enqueueThreadId_.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
int BenchmarkWrapper<DataStructure, TOrder>::addDequeueThread(){
    return dequeueThreadId_.fetch_add(1, std::memory_order_relaxed);
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::enqueue_order(TOrder &o, int threadId) {
    // start timer
    uint64_t t0 = lTime::rdtscp_inline();
    // enqueue datastructure
    bool enqueued = structure_.enqueue(o);
    // end timer
    uint64_t t1 = lTime::rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + localIndexEnq_[threadId]++; // per-producer index
    latencies_enqueue[idx] = delta;
    return enqueued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeue_latency(TOrder &o, int threadId) {
    uint64_t t0 = lTime::rdtscp_inline();
    // dequeue datastructure
    bool dequeued = structure_.dequeue(o);
    // end timer
    uint64_t t1 = lTime::rdtscp_inline();
    // maybe add logic for if transaction wasn't successful?

    // calculations and storing
    uint64_t delta = t1 - t0;
    uint64_t idx = threadId * THREAD_LIMIT_ + localIndexDeq_[threadId]++; // per-consumer index
    latencies_dequeue[idx] = delta;
    return dequeued;
}

template<typename DataStructure, typename TOrder>
bool BenchmarkWrapper<DataStructure, TOrder>::dequeue_ordering(TOrder &o, int threadId) {
    bool dequeued = structure_.dequeue(o);
    uint64_t idx = threadId * THREAD_LIMIT_ + localIndexDeq_[threadId]++;

    sequence_dequeue[idx] = o.sequence_number;
    timestamps_dequeue[idx] = lTime::rdtscp_inline();

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
        double e_ns = latencies_enqueue[i] / cycles_per_ns;
        sumEnq += e_ns;
        e_ns_latencies.push_back(e_ns);

        double d_ns = latencies_dequeue[i] / cycles_per_ns;
        sumDeq += d_ns;
        d_ns_latencies.push_back(d_ns);
    }
    sumEnq /= TOTAL_ORDERS_;
    sumDeq /= TOTAL_ORDERS_;

    std::cout << "Avg Enqueue Latency (ns): " << sumEnq << std::endl;
    std::cout << "Avg Dequeue Latency (ns): " << sumDeq << std::endl;
    latencies::write_csv_latencies(e_ns_latencies, d_ns_latencies);
}

template<typename DataStructure, typename TOrder>
void BenchmarkWrapper<DataStructure, TOrder>::processOrders(CollectionOrderGenerator<BenchmarkOrder> &generator){
    std::vector<uint64_t> actual_order; // moving to a vector to reduce segfaults and stuff
    std::vector<uint64_t> expected_order;


    std::vector<std::pair<uint64_t, uint64_t>> t_s; // timestamp, then sequence
    t_s.reserve(TOTAL_ORDERS_);

    for (size_t i = 0; i < TOTAL_ORDERS_; ++i) {
        t_s.emplace_back(timestamps_dequeue[i], sequence_dequeue[i]); // make pair of each element, then sort by timestamp
    }

    std::sort(t_s.begin(), t_s.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first;
        }
    );

    for (size_t i = 0; i < TOTAL_ORDERS_; ++i){
        actual_order.push_back(t_s[i].second);
    }

    for (size_t i = 0; i < TOTAL_ORDERS_; ++i){
        // std::cout << "Order sequence: " << sequence_dequeue[i] << std::endl;
        TOrder order = generator.generate();
        expected_order.push_back(order.order_id);
    }
    ordering::write_csv_ordering(expected_order, actual_order);
}

template class BenchmarkWrapper<RegularQueue<BenchmarkOrder>, BenchmarkOrder>;
template class BenchmarkWrapper<MCLockFreeQueue<BenchmarkOrder>, BenchmarkOrder>;
template class BenchmarkWrapper<MCConcurrentQueue<BenchmarkOrder>, BenchmarkOrder>;