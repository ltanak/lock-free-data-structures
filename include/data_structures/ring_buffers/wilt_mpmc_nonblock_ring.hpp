#pragma once

#include "data_structures/ring_buffers/i_ring_buffer.hpp"
#include "data_structures/ring_buffers/ring.h"

/**
 * @class WiltMPMCNonBlockRing
 * @brief multi-producer, multi-consumer non-blocking ring buffer.
 * @tparam TOrder Type of orders stored in the queue
 */
template <typename TOrder>
class WiltMPMCNonBlockRing : public IRing<TOrder, WiltMPMCNonBlockRing<TOrder>> {

public:
    WiltMPMCNonBlockRing(size_t capacity = 100000);
    auto enqueueOrder(TOrder &order) -> bool;
    auto dequeueOrder(TOrder &order) -> bool;
    auto getSize() -> uint64_t;
    auto isEmpty() -> bool;
    auto getFront(TOrder &order) -> bool;

private:
    wilt::Ring<TOrder> wilt_ring_buffer_;
};
