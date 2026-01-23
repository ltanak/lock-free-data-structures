#pragma once

#include <vector>
#include <memory>

/**
 * louis-logs structs, representing the aligned structs
 * that store data collected during testing scenarios
 */

namespace llogs {

    struct alignas(64) LatencyStore {
        std::unique_ptr<uint64_t> enqueue_buffers;
        std::unique_ptr<uint64_t> dequeue_buffers;
    };

    struct alignas(64) OrderingStore {
        std::unique_ptr<uint64_t> sequence_buffers;
        std::unique_ptr<uint64_t> timestamp_buffers;
    };

}