#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "utils/files.hpp"
#include "utils/timing.hpp"
#include "scenarios/test_inputs.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/market_state.hpp"

// anonymous namespace reduces symbol leaks, and makes makeOrder only defined within this file (can use static, but namespace is modern C++)
namespace {

    BenchmarkOrder makeOrder(uint64_t id = 1, OrderType type = OrderType::BUY, double price = 10.0, uint32_t qty = 5.0, uint64_t seq = 0) {
		uint64_t ts = lTime::rdtscp_inline();
        return BenchmarkOrder{id, type, price, qty, ts, seq};
    }

}

TEST(RegularQueueTest, EnqueueDequeueAndFront) {
	RegularQueue<BenchmarkOrder> q;
	BenchmarkOrder in = makeOrder(42, OrderType::SELL, 25.5, 3.0, 7);
	BenchmarkOrder out{};
	BenchmarkOrder front{};

	EXPECT_TRUE(q.empty());
	EXPECT_EQ(q.size(), 0u);

	EXPECT_TRUE(q.enqueueOrder(in));
	EXPECT_FALSE(q.empty());
	EXPECT_EQ(q.size(), 1u);

	EXPECT_TRUE(q.getFront(front));
	EXPECT_EQ(front.order_id, in.order_id);
	EXPECT_EQ(front.type, in.type);
	EXPECT_DOUBLE_EQ(front.price, in.price);
	EXPECT_DOUBLE_EQ(front.quantity, in.quantity);
	EXPECT_EQ(front.sequence_number, in.sequence_number);

	EXPECT_TRUE(q.dequeueOrder(out));
	EXPECT_EQ(out.order_id, in.order_id);
	EXPECT_TRUE(q.empty());
	EXPECT_EQ(q.size(), 0u);
}

TEST(RegularQueueTest, DequeueEmptyReturnsFalse) {
	RegularQueue<BenchmarkOrder> q;
	BenchmarkOrder out{};
	EXPECT_FALSE(q.dequeueOrder(out));
	EXPECT_TRUE(q.isEmpty());
}

TEST(MCLockFreeQueueTest, BasicOperations) {
	MCLockFreeQueue<BenchmarkOrder> q;
	BenchmarkOrder in = makeOrder(11, OrderType::BUY, 15.0, 1.0, 1);
	BenchmarkOrder out{};
	BenchmarkOrder front{};

	EXPECT_TRUE(q.isEmpty());
	EXPECT_EQ(q.getSize(), 0u);

	EXPECT_TRUE(q.enqueueOrder(in));
	EXPECT_FALSE(q.isEmpty());
	EXPECT_EQ(q.getSize(), 1u);

	EXPECT_TRUE(q.getFront(front));
	EXPECT_EQ(front.order_id, in.order_id);

	EXPECT_TRUE(q.dequeueOrder(out));
	EXPECT_EQ(out.order_id, in.order_id);
	EXPECT_TRUE(q.isEmpty());
}

TEST(MCConcurrentQueueTest, BasicOperations) {
	MCConcurrentQueue<BenchmarkOrder> q;
	BenchmarkOrder in = makeOrder(21, OrderType::SELL, 30.0, 2.5, 9);
	BenchmarkOrder out{};

	EXPECT_TRUE(q.isEmpty());
	EXPECT_TRUE(q.enqueueOrder(in));
	EXPECT_EQ(q.getSize(), 1u);

	EXPECT_TRUE(q.dequeueOrder(out));
	EXPECT_EQ(out.order_id, in.order_id);
	EXPECT_TRUE(q.isEmpty());
	EXPECT_EQ(q.getSize(), 0u);
}

TEST(MCConcurrentQueueTest, GetFrontThrowsNotImplemented) {
	MCConcurrentQueue<BenchmarkOrder> q;
	BenchmarkOrder in = makeOrder();
	q.enqueueOrder(in);
	BenchmarkOrder front{};
	EXPECT_THROW(q.getFront(front), std::logic_error);
}

TEST(TimingTest, MeasureTscGhzReasonableRange) {
	double ghz = lTime::measure_tsc_ghz();
	EXPECT_GT(ghz, 0.1);   // should be positive
	EXPECT_LT(ghz, 10.0);  // typical CPU upper bound guard
}

TEST(TestInputsTest, ParseArgsFillsParams) {
	TestParams params{};
	const char *argv[] = {"prog", "stress", "4", "100"};
	parseArgs(4, const_cast<char **>(argv), params);

	EXPECT_EQ(params.test, TestType::STRESS);
	EXPECT_EQ(params.thread_count, 4u);
	EXPECT_EQ(params.thread_order_limit, 100u);
	EXPECT_EQ(params.total_orders, 400u);
}