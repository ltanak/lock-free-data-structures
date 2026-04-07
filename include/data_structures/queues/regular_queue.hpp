#pragma once

#include <queue>
#include <mutex>
#include <random>
#include <chrono>
#include <optional>
#include "data_structures/queues/i_queue.hpp"

/**
 * @class RegularQueue
 * @brief OS-locking implementation of the regular std::queue
 * @tparam TOrder Type of orders stored in the queue
 */
template <typename TOrder>
class RegularQueue : public IQueue<TOrder, RegularQueue<TOrder>> {

public:
    RegularQueue();
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;

private:
    uint64_t size_;
    mutable std::mutex mutexLock_;
    std::queue<TOrder> queue_;
};
