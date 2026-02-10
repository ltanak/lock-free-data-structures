#pragma once
#include <atomic>
#include <random>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <map>

/**
 *  - 15% volatility
 * MEDIUM = 15 - 25%
 * HIGH = 25 - 35%  
 */

enum class Volatility {
    LOW, MEDIUM, HIGH
};

enum class MarketEvent {
    BASE,
    UP_TREND,
    DOWN_TREND,
    CRASH,
    PULL_BACK,
    SURGE
};

struct EventParams {
    Volatility volatility;
    double bias;
    double drift;
    double spread;
    uint32_t event_duration; // counter of how many cycles the event should last for
};

/**
 * @brief each generator reads from the market state to inform its order generation
 * atomic variables for thread-safe access and modification
 * aligned to cache line size (prevents false sharing)
 */

class MarketState {
public:
    explicit MarketState(std::optional<uint32_t> seed = std::nullopt);
    
    double getPrice() const noexcept;
    double getVolatility() const noexcept;
    double getBias() const noexcept;
    double getDrift() const noexcept;
    double getSpread() const noexcept;
    MarketEvent getEvent() const noexcept;
    
    // events
    void applyEvent(MarketEvent event_type) noexcept;
    void applyEvent(MarketEvent evt, uint64_t current_cycle) noexcept;
    void setParameters(EventParams params) noexcept;
    void checkEvent(uint64_t cycle_count) noexcept;
    void checkAndApplyEvent(uint64_t cycle_count) noexcept;
    void updatePrice() noexcept;
    double sampleVolatility(Volatility vol) noexcept;
    const EventParams& getEventParams(MarketEvent evt) const noexcept;

private:
    std::atomic<double> base_price_{100.0};
    std::atomic<double> volatility_{0.2};
    std::atomic<double> bias_{0.0}; // who has "priority" or who is more aggressive, buyers or sellers
    std::atomic<double> drift_{0.0}; // general market projection
    std::atomic<double> spread_{0.02};
    std::atomic<Volatility> curr_volatility_{Volatility::LOW};
    std::atomic<MarketEvent> event_{MarketEvent::BASE};

    static constexpr std::array<std::pair<MarketEvent, EventParams>, 6> event_params{
        std::pair{MarketEvent::BASE, EventParams{Volatility::LOW, 0.00, 0.5, 0.02, 0}},
        std::pair{MarketEvent::UP_TREND, EventParams{Volatility::LOW, 0.01, 0.55, 0.02, 150}},
        std::pair{MarketEvent::DOWN_TREND, EventParams{Volatility::LOW, -0.01, 0.45, 0.02, 150}},
        std::pair{MarketEvent::SURGE, EventParams{Volatility::MEDIUM, 0.03, 0.65, 0.025, 100}},
        std::pair{MarketEvent::PULL_BACK, EventParams{Volatility::MEDIUM, -0.03, 0.35, 0.025, 100}},
        std::pair{MarketEvent::CRASH, EventParams{Volatility::MEDIUM, -0.05, 0.25, 0.03, 80}},
    };
    
    // state for event generation
    std::mt19937 rng_;
    uint64_t cycle_count_{0};
    uint64_t last_event_cycle_{0};
    std::atomic<uint64_t> event_duration_{0};
    std::atomic<uint64_t> event_end_{0};
    // starts running
    // once we are past the current count we load an event
    // we mark the event duration
    // once we go past the event duration we reset back to base
    
    //volatility distribution
    std::uniform_int_distribution<int> low_vol_dist_{3, 6};     // 3 - 6%
    std::uniform_int_distribution<int> med_vol_dist_{6, 10};    // 6 - 10%
    std::uniform_int_distribution<int> high_vol_dist_{10, 15};  // 10 - 15%

};
