#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include "data_structures/queues/regular_queue.hpp"
#include "data_structures/queues/mc_lockfree_queue.hpp"
#include "data_structures/queues/mc_mpmc_queue.hpp"
#include "utils/files.hpp"
#include "utils/timing.hpp"
#include "scenarios/test_inputs.hpp"
#include "order_simulation/benchmark_order.hpp"
#include "order_simulation/market_state.hpp"
#include "exchange/matching_engine.hpp"

// anonymous namespace reduces symbol leaks, and makes makeOrder only defined within this file (can use static, but namespace is modern C++)
namespace {

    BenchmarkOrder makeOrder(uint64_t id = 1, OrderType type = OrderType::BUY, double price = 10.0, uint32_t qty = 5, uint64_t seq = 0) {
		uint64_t ts = ltime::rdtscp_inline();
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
	BenchmarkOrder in = makeOrder(21, OrderType::SELL, 30.0, 2, 9);
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

TEST(MatchingEngineTest, EnforcesPriceTimePriority) {
	MatchingEngine<BenchmarkOrder> engine(100.0);

	auto sell_old = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::SELL, 100.50, 10, 1)));
	auto sell_new = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::SELL, 100.50, 10, 2)));

	engine.processOrder(sell_old.get());
	engine.processOrder(sell_new.get());

	auto buy = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(3, OrderType::BUY, 101.00, 15, 3)));
	engine.processOrder(buy.get());

	int level_index = engine.sell_book.priceToIndex(sell_old->price_ticks);
	PriceLevel &level = engine.sell_book.levels[level_index];

	EXPECT_EQ(sell_old->quantity, 0u);
	EXPECT_EQ(sell_new->quantity, 5u);
	EXPECT_EQ(level.head, sell_new.get());
	EXPECT_EQ(level.tail, sell_new.get());
	EXPECT_EQ(level.total_quantity, 5);
	EXPECT_EQ(engine.sell_book.bestPriceLevel(), level_index);
	EXPECT_EQ(buy->quantity, 0u);
}

TEST(MatchingEngineTest, MultipleOrdersTest){
	MatchingEngine<BenchmarkOrder> engine(100.0);

	// adding multiple sell orders
	auto sell1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::SELL, 101.00, 5, 1)));
	auto sell2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::SELL, 101.00, 7, 2)));
	auto sell3 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(3, OrderType::SELL, 102.00, 10, 3)));

	engine.processOrder(sell1.get());
	engine.processOrder(sell2.get());
	engine.processOrder(sell3.get());

	// buy order that crosses sell1 and sell2 at 101.00
	auto buy1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(4, OrderType::BUY, 101.50, 8, 4)));
	engine.processOrder(buy1.get());

	// buy1 should match against sell1 (5 qty) and partially against sell2 (3 qty)
	EXPECT_EQ(buy1->quantity, 0u);
	EXPECT_EQ(sell1->quantity, 0u);
	EXPECT_EQ(sell2->quantity, 4u);

	// next buy order that crosses remaining sells
	auto buy2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(5, OrderType::BUY, 102.50, 20, 5)));
	engine.processOrder(buy2.get());

	// buy2 should match against remaining sell2 (4 qty) and sell3 (10 qty), leaving 6 qty
	EXPECT_EQ(buy2->quantity, 6u);
	EXPECT_EQ(sell2->quantity, 0u);
	EXPECT_EQ(sell3->quantity, 0u);

	// check buy2 is now in the buy book
	int buy_level = engine.buy_book.bestPriceLevel();
	EXPECT_GE(buy_level, 0);
	EXPECT_EQ(engine.buy_book.levels[buy_level].total_quantity, 6u);
	EXPECT_EQ(engine.buy_book.levels[buy_level].head->quantity, 6u);
}

TEST(MatchingEngineTest, RecenteringAndShifting) {
	MatchingEngine<BenchmarkOrder> engine(100.0);

	auto sell1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::SELL, 101.00, 10, 1)));
	auto sell2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::SELL, 102.00, 5, 2)));
	
	engine.processOrder(sell1.get());
	engine.processOrder(sell2.get());

	// check internal state / priceLevels and indexes
	EXPECT_EQ(engine.sell_book.centre_price_ticks, engine.priceToTicks(100.0));
	int sell_level_1_before = engine.sell_book.priceToIndex(sell1->price_ticks);
	int sell_level_2_before = engine.sell_book.priceToIndex(sell2->price_ticks);
	EXPECT_GE(sell_level_1_before, 0);
	EXPECT_GE(sell_level_2_before, 0);

	// check both orders are in the book with correct total quantity
	EXPECT_EQ(engine.sell_book.levels[sell_level_1_before].total_quantity, 10u);
	EXPECT_EQ(engine.sell_book.levels[sell_level_2_before].total_quantity, 5u);

	// recentre the books to a new price (110.0) - still close to original
	engine.sell_book.recentre(engine.priceToTicks(110.0));

	// centre has moved
	EXPECT_EQ(engine.sell_book.centre_price_ticks, engine.priceToTicks(110.0));

	// orders are still at the correct price levels after recentering
	int sell_level_1_after = engine.sell_book.priceToIndex(sell1->price_ticks);
	int sell_level_2_after = engine.sell_book.priceToIndex(sell2->price_ticks);
	EXPECT_GE(sell_level_1_after, 0);
	EXPECT_GE(sell_level_2_after, 0);

	// orders still have their quantities after recentering
	EXPECT_EQ(engine.sell_book.levels[sell_level_1_after].total_quantity, 10u);
	EXPECT_EQ(engine.sell_book.levels[sell_level_2_after].total_quantity, 5u);

	// check bitmap is set correctly for both orders
	uint64_t sell_word_1 = sell_level_1_after / 64;
	uint64_t sell_bit_1 = sell_level_1_after % 64;
	EXPECT_TRUE((engine.sell_book.bitmap[sell_word_1] & (1ULL << sell_bit_1)));

	uint64_t sell_word_2 = sell_level_2_after / 64;
	uint64_t sell_bit_2 = sell_level_2_after % 64;
	EXPECT_TRUE((engine.sell_book.bitmap[sell_word_2] & (1ULL << sell_bit_2)));
}

TEST(TimingTest, MeasureTscGhzReasonableRange) {
	double ghz = ltime::measure_tsc_ghz();
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