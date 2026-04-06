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
		uint64_t ts = ltime::rdtsc_lfence();
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

TEST(RegularQueueTest, MultipleEnqueueDequeue) {
	RegularQueue<BenchmarkOrder> q;
	std::vector<BenchmarkOrder> orders;
	
	for (int i = 0; i < 10; ++i) {
		orders.push_back(makeOrder(i, OrderType::BUY, 100.0 + i, i + 1, i));
		EXPECT_TRUE(q.enqueueOrder(orders[i]));
	}
	
	EXPECT_EQ(q.size(), 10u);
	
	for (int i = 0; i < 10; ++i) {
		BenchmarkOrder out{};
		EXPECT_TRUE(q.dequeueOrder(out));
		EXPECT_EQ(out.order_id, i);
		EXPECT_EQ(out.sequence_number, i);
	}
	
	EXPECT_TRUE(q.empty());
}

TEST(RegularQueueTest, AlternateEnqueueDequeue) {
	RegularQueue<BenchmarkOrder> q;
	
	auto order1 = makeOrder(1, OrderType::SELL, 50.0, 10, 0);
	EXPECT_TRUE(q.enqueueOrder(order1));
	
	BenchmarkOrder out1{};
	EXPECT_TRUE(q.dequeueOrder(out1));
	EXPECT_EQ(out1.order_id, 1);
	
	auto order2 = makeOrder(2, OrderType::BUY, 60.0, 20, 1);
	EXPECT_TRUE(q.enqueueOrder(order2));
	
	BenchmarkOrder out2{};
	EXPECT_TRUE(q.dequeueOrder(out2));
	EXPECT_EQ(out2.order_id, 2);
	
	EXPECT_TRUE(q.empty());
}

// MCLockFreeQueue

TEST(MCLockFreeQueueTest, MultipleEnqueueDequeue) {
	MCLockFreeQueue<BenchmarkOrder> q;
	
	for (int i = 0; i < 20; ++i) {
		auto order = makeOrder(i, OrderType::BUY, 100.0 + i, i + 1, i);
		EXPECT_TRUE(q.enqueueOrder(order));
	}
	
	EXPECT_EQ(q.getSize(), 20u);
	
	for (int i = 0; i < 20; ++i) {
		BenchmarkOrder out{};
		EXPECT_TRUE(q.dequeueOrder(out));
		EXPECT_EQ(out.order_id, i);
	}
	
	EXPECT_TRUE(q.isEmpty());
}

TEST(MCLockFreeQueueTest, GetFrontConsistency) {
	MCLockFreeQueue<BenchmarkOrder> q;
	
	auto order1 = makeOrder(100, OrderType::SELL, 200.0, 50, 5);
	EXPECT_TRUE(q.enqueueOrder(order1));
	
	BenchmarkOrder front1{}, front2{};
	EXPECT_TRUE(q.getFront(front1));
	EXPECT_TRUE(q.getFront(front2));
	EXPECT_EQ(front1.order_id, front2.order_id);
	EXPECT_EQ(front1.order_id, 100);
}

// MCConcurrentQueue

TEST(MCConcurrentQueueTest, LargeSequence) {
	MCConcurrentQueue<BenchmarkOrder> q;
	
	for (int i = 0; i < 100; ++i) {
		auto order = makeOrder(i, (i % 2 == 0) ? OrderType::BUY : OrderType::SELL, 
							   100.0 + (i % 20), i + 1, i);
		EXPECT_TRUE(q.enqueueOrder(order));
	}
	
	EXPECT_EQ(q.getSize(), 100u);
	
	for (int i = 0; i < 100; ++i) {
		BenchmarkOrder out{};
		EXPECT_TRUE(q.dequeueOrder(out));
		EXPECT_EQ(out.order_id, i);
	}
	
	EXPECT_TRUE(q.isEmpty());
	EXPECT_EQ(q.getSize(), 0u);
}

// MatchingEngine

TEST(MatchingEngineTest, PartialFillScenario) {
	MatchingEngine<BenchmarkOrder> engine(100.0);
	
	// sell 50 units at 100.50
	auto sell = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::SELL, 100.50, 50, 1)));
	engine.processOrder(sell.get());
	
	// buy 30 units at 100.60 (crosses)
	auto buy1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::BUY, 100.60, 30, 2)));
	engine.processOrder(buy1.get());
	
	EXPECT_EQ(buy1->quantity, 0u);    // fully matched
	EXPECT_EQ(sell->quantity, 20u);   // partially filled
	
	// buy remaining 20 units
	auto buy2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(3, OrderType::BUY, 100.60, 20, 3)));
	engine.processOrder(buy2.get());
	
	EXPECT_EQ(buy2->quantity, 0u);
	EXPECT_EQ(sell->quantity, 0u);
}

TEST(MatchingEngineTest, NoCrossAtDifferentPrices) {
	MatchingEngine<BenchmarkOrder> engine(100.0);
	
	// sell at 101.00
	auto sell = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::SELL, 101.00, 10, 1)));
	engine.processOrder(sell.get());
	
	// buy at 100.00 (below sell, no cross)
	auto buy = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::BUY, 100.00, 5, 2)));
	engine.processOrder(buy.get());
	
	EXPECT_EQ(buy->quantity, 5u);    // not filled
	EXPECT_EQ(sell->quantity, 10u);  // not filled
}

TEST(MatchingEngineTest, BuyTakesSellAtMultipleLevels) {
	MatchingEngine<BenchmarkOrder> engine(100.0);
	
	// sells at different price levels
	auto sell1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::SELL, 100.50, 5, 1)));
	auto sell2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::SELL, 101.00, 8, 2)));
	auto sell3 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(3, OrderType::SELL, 101.50, 10, 3)));
	
	engine.processOrder(sell1.get());
	engine.processOrder(sell2.get());
	engine.processOrder(sell3.get());
	
	// buy at aggressive price that crosses multiple levels
	auto buy = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(4, OrderType::BUY, 102.00, 20, 4)));
	engine.processOrder(buy.get());
	
	// sell1 (5) + sell2 (8) + partial sell3 (7)
	EXPECT_EQ(sell1->quantity, 0u);
	EXPECT_EQ(sell2->quantity, 0u);
	EXPECT_EQ(sell3->quantity, 3u);
	EXPECT_EQ(buy->quantity, 0u);
}

TEST(MatchingEngineTest, SelfTradePreventionSameBuyLevel) {
	MatchingEngine<BenchmarkOrder> engine(100.0);
	
	// add two buy orders at same price level
	auto buy1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(1, OrderType::BUY, 100.00, 10, 1)));
	auto buy2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(2, OrderType::BUY, 100.00, 5, 2)));
	
	engine.processOrder(buy1.get());
	engine.processOrder(buy2.get());
	
	// add opposing sell - should only match with older buy (buy1 by sequence)
	auto sell = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(3, OrderType::SELL, 100.00, 8, 3)));
	engine.processOrder(sell.get());
	
	EXPECT_EQ(buy1->quantity, 2u);   // matched 8 from its 10
	EXPECT_EQ(buy2->quantity, 5u);   // not matched yet
	EXPECT_EQ(sell->quantity, 0u);   // fully matched
}

// PriceBook and PriceLevel Tests

TEST(PriceLevelTest, OrderingBySequenceNumber) {
	MatchingEngine<BenchmarkOrder> engine(100.0);
	
	// add three orders at same price with different sequence numbers
	auto order1 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(10, OrderType::BUY, 100.00, 5, 1)));
	auto order2 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(11, OrderType::BUY, 100.00, 3, 3)));
	auto order3 = std::make_unique<BookOrder>(engine.convertOrder(makeOrder(12, OrderType::BUY, 100.00, 7, 2)));
	
	engine.processOrder(order1.get());
	engine.processOrder(order2.get());
	engine.processOrder(order3.get());
	
	// query the price level
	int level = engine.buy_book.priceToIndex(order1->price_ticks);
	PriceLevel &pl = engine.buy_book.levels[level];
	
	// head should be order1 (sequence 1, earliest)
	EXPECT_EQ(pl.head->order_id, 10);
	EXPECT_EQ(pl.total_quantity, 15u);  // 5 + 3 + 7
}

// Conversion and Price Encoding Tests

TEST(MatchingEngineTest, PriceTickConversion) {
	MatchingEngine<BenchmarkOrder> engine(100.0);
	
	double price1 = 100.50;
	double price2 = 100.75;
	
	int64_t ticks1 = engine.priceToTicks(price1);
	int64_t ticks2 = engine.priceToTicks(price2);
	
	// ticks should be different for different prices
	EXPECT_NE(ticks1, ticks2);
	
	// conversion back should be close to original (accounting for tick precision)
	double restored1 = engine.ticksToPrice(ticks1);
	EXPECT_NEAR(restored1, price1, 0.001);
}

TEST(MatchingEngineTest, DeterministicOutput) {
	/**
	 * Runs the same order sequence through two ME instances
	 * asserts equal outcomes
	 */
    auto run = [](int seed_offset) {
        MatchingEngine<BenchmarkOrder> engine(100.0);

		// 
        auto sell_1 = std::make_unique<BookOrder>(
            engine.convertOrder(makeOrder(1, OrderType::SELL, 101.00, 10, 1)));
        auto sell_2 = std::make_unique<BookOrder>(
            engine.convertOrder(makeOrder(2, OrderType::SELL, 102.00, 5,  2)));
        auto buy_1 = std::make_unique<BookOrder>(
            engine.convertOrder(makeOrder(3, OrderType::BUY,  101.50, 8,  3)));
        auto buy_2 = std::make_unique<BookOrder>(
            engine.convertOrder(makeOrder(4, OrderType::BUY,  102.50, 10, 4)));
        engine.processOrder(sell_1.get());
        engine.processOrder(sell_2.get());
        engine.processOrder(buy_1.get());
        engine.processOrder(buy_2.get());

		// engine updates fields in-place
        return std::make_tuple(
            sell_1->quantity, sell_2->quantity,
            buy_1->quantity, buy_2->quantity
        );
    };

    auto result1 = run(0);
    auto result2 = run(0);

    EXPECT_EQ(std::get<0>(result1), std::get<0>(result2));  // sell1 remaining
    EXPECT_EQ(std::get<1>(result1), std::get<1>(result2));  // sell2 remaining
    EXPECT_EQ(std::get<2>(result1), std::get<2>(result2));  // buy1 remaining
    EXPECT_EQ(std::get<3>(result1), std::get<3>(result2));  // buy2 remaining
}