#pragma once

#include "data_structures/queues/i_queue.hpp"
#include "data_structures/queues/concurrentqueue.h"

template <typename TOrder>
class MCConcurrentQueue : public IQueue<TOrder, MCConcurrentQueue<TOrder>> {

public:
    MCConcurrentQueue();
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;

private:
    moodycamel::ConcurrentQueue<TOrder> mcMPMCQueue_;
};
