#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "order_simulation/order.hpp"
#include <cstdint>

template<typename TOrder>
MCLockFreeQueue<TOrder>::MCLockFreeQueue(): size_(0) {}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::enqueueOrder(TOrder &order){
    return mcQueue_.enqueue(order);
}

template<typename TOrder>
void MCLockFreeQueue<TOrder>::dequeueOrder(){
    TOrder order;
    bool success = mcQueue_.try_dequeue(order);
    if (!success){
        // will do a terminal logging error saying not possible
    } 
}

template<typename TOrder>
uint64_t MCLockFreeQueue<TOrder>::getSize(){
    return size_;
}

template<typename TOrder>
TOrder MCLockFreeQueue<TOrder>::getFront(){
    TOrder *order = mcQueue_.peek();
    return *order;
}

template<typename TOrder>
bool MCLockFreeQueue<TOrder>::isEmpty(){
    return size_ == 0;
}

template class MCLockFreeQueue<Order>;