#pragma once

#include <cstdint>

/**
 * @brief Interface for queue data structures
 * @tparam TOrder The type of the order being pushed to the queue
 * Using CRTP instead of virtual functions for performance reasons
 * Every derived class implements its own generateOrder method
 */
template <typename TOrder, typename QueueImpl>
class IQueue {
    public:
        // we pass by reference so that we can point to the value that has been dequeued
        bool enqueue(TOrder &order){
            return static_cast<QueueImpl*>(this)->enqueueOrder(order);
        }
        bool dequeue(TOrder &order){
            return static_cast<QueueImpl*>(this)->dequeueOrder(order);
        }
        uint64_t size(){
            return static_cast<QueueImpl*>(this)->getSize();
        }
        bool empty(){
            return static_cast<QueueImpl*>(this)->isEmpty();
        }
        bool front(TOrder &order){
            return static_cast<QueueImpl*>(this)->getFront(order);
        }
};
