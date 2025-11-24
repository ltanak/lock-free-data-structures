#pragma once

#include <queue>
#include <mutex>
#include <random>
#include <chrono>
#include <optional>
#include "data_structures/queues/i_queue.hpp"

template <typename TOrder>
class RegularQueue : public IQueue<TOrder, RegularQueue<TOrder>> {

public:
    RegularQueue();
    bool enqueueOrder(TOrder &order);
    TOrder dequeueOrder();
    uint64_t getSize();
    bool isEmpty();

private:
    uint64_t size_;
    mutable std::mutex mutex_;
    std::queue<TOrder> queue_;
};