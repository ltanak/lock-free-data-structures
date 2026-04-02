#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/rigtorp_mpmc_queue.hpp"
#include "order_simulation/benchmark_order.hpp"
#include <cstdint>

#define NOT_IMPLEMENTED throw std::logic_error("Function not implemented.");

template<typename TOrder>
RigtorpMPMCQueue<TOrder>::RigtorpMPMCQueue(size_t capacity): 
    rigtorp_queue_(capacity) {
}

template<typename TOrder>
bool RigtorpMPMCQueue<TOrder>::enqueueOrder(TOrder &order){
    rigtorp_queue_.try_emplace(order);
    return true;
}

template<typename TOrder>
bool RigtorpMPMCQueue<TOrder>::dequeueOrder(TOrder &order){
    rigtorp_queue_.try_pop(order);
    return true;
}

template<typename TOrder>
uint64_t RigtorpMPMCQueue<TOrder>::getSize(){
    ssize_t size = rigtorp_queue_.size();
    return (size < 0) ? 0 : size;
}

template<typename TOrder>
bool RigtorpMPMCQueue<TOrder>::getFront(TOrder &order){
    NOT_IMPLEMENTED
    return false;
}

template<typename TOrder>
bool RigtorpMPMCQueue<TOrder>::isEmpty(){
    return rigtorp_queue_.empty();
}

template class RigtorpMPMCQueue<BenchmarkOrder>;
