#pragma once

/**
 * @brief Interface for queue data structures
 * @tparam TOrder The type of the order being pushed to the queue
 * Using CRTP instead of virtual functions for performance reasons
 * Every derived class implements its own generateOrder method
 */
template <typename TOrder, typename QueueImpl>
class IQueue {
    public:
        bool enqueue(TOrder &order){
            return static_cast<QueueImpl*>(this)->enqueueOrder();
        }
        TOrder dequeue(){
            return static_cast<QueueImpl*>(this)->dequeueOrder();
        }
        uint64_t size(){
            return static_cast<QueueImpl*>(this)->getSize();
        }
        bool empty(){
            return static_cast<QueueImpl*>(this)->isEmpty();
        }
};
