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
        auto enqueue(TOrder &order) -> bool {
            return static_cast<QueueImpl*>(this)->enqueueOrder(order);
        }
        auto dequeue(TOrder &order) -> bool {
            return static_cast<QueueImpl*>(this)->dequeueOrder(order);
        }
        auto size() -> uint64_t{
            return static_cast<QueueImpl*>(this)->getSize();
        }
        auto empty() -> bool{
            return static_cast<QueueImpl*>(this)->isEmpty();
        }
        auto front(TOrder &order) -> bool {
            return static_cast<QueueImpl*>(this)->getFront(order);
        }
};
