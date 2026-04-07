#pragma once

#include <cstdint>

/**
 * @brief Interface for the lock free ring buffers
 * @tparam TOrder the type of the order being pushed to the ring buffer
 * Each derived class implements this interface
 */
template<typename TOrder, typename RingImpl>
class IRing {
public:
    auto enqueue(TOrder &order) -> bool {
        return static_cast<RingImpl*>(this)->enqueueOrder(order);
    }
    auto dequeue(TOrder &order) -> bool {
        return static_cast<RingImpl*>(this)->dequeueOrder(order);
    }
    auto size() -> uint64_t{
        return static_cast<RingImpl*>(this)->getSize();
    }
    auto empty() -> bool{
        return static_cast<RingImpl*>(this)->isEmpty();
    }
    auto front(TOrder &order) -> bool {
        return static_cast<RingImpl*>(this)->getFront(order);
    }
};
