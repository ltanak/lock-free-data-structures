#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "order_simulation/benchmark_order.hpp"
#include <cstdint>

template<typename TOrder>
MCLockFreeQueue<TOrder>::MCLockFreeQueue(): size_(0) {}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::enqueueOrder(TOrder &order){
    if (mcQueue_.enqueue(order)){
        size_++;
        return true;
    }
    return false;
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::dequeueOrder(TOrder &order){
    if (!mcQueue_.try_dequeue(order)){
        // will do a terminal logging error saying not possible
        return false;
    } else {
        size_--;
        return true;
    }
}

template<typename TOrder>
uint64_t MCLockFreeQueue<TOrder>::getSize(){
    return size_;
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::getFront(TOrder &order){
    if (auto *ptr = mcQueue_.peek()){
        order = *ptr;
        return true;
    }
    return false;
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::isEmpty(){
    return size_ == 0;
}

template class MCLockFreeQueue<BenchmarkOrder>;