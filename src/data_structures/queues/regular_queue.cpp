#include <queue>
#include <mutex>
#include <random>
#include <chrono>
#include <optional>
#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/regular_queue.hpp"
#include "order_simulation/order.hpp"

template<typename TOrder>
RegularQueue<TOrder>::RegularQueue(): size_(0) {}

template<typename TOrder>
bool RegularQueue<TOrder>::enqueueOrder(TOrder &order){
    std::lock_guard<std::mutex> lock(mutexLock_);
    queue_.push(order);
    size_++;
    return true;
}

template<typename TOrder>
void RegularQueue<TOrder>::dequeueOrder(){
    std::lock_guard<std::mutex> lock(mutexLock_);
    if (!queue_.empty()){
        queue_.pop();
        size_--;
    }
}

template<typename TOrder>
TOrder RegularQueue<TOrder>::dequeueOrderV(){
    std::lock_guard<std::mutex> lock(mutexLock_);
    TOrder order; // check this
    if (!queue_.empty()){
        order = queue_.front();
        queue_.pop();
        size_--;
    }
    return order;
}

template<typename TOrder>
uint64_t RegularQueue<TOrder>::getSize(){
    std::lock_guard<std::mutex> lock(mutexLock_);
    return size_;
}

template<typename TOrder>
bool RegularQueue<TOrder>::isEmpty(){
    std::lock_guard<std::mutex> lock(mutexLock_);
    if (size_ == 0){
        return true;
    }
    return false;
}

template<typename TOrder>
TOrder RegularQueue<TOrder>::getFront(){
    std::lock_guard<std::mutex> lock(mutexLock_);
    TOrder order; // check this
    if (!queue_.empty()){
        order = queue_.front();
        queue_.pop();
        size_--;
    }
    return order;
}

template class RegularQueue<Order>;