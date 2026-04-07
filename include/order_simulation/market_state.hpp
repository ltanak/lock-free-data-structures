#pragma once
#include <atomic>
#include <random>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <map>

/**
 * Volatility regimes (per-tick standard deviation as fraction of price)
 * LOW    = ~0.01 - 0.03% per tick  (calm markets)
 * MEDIUM = ~0.03 - 0.08% per tick  (active trading)
 * HIGH   = ~0.08 - 0.20% per tick  (event-driven / crisis)
 */
enum class Volatility {
    LOW, MEDIUM, HIGH
};

/**
 * Different types of market events implemented
 */
enum class MarketEvent {
    BASE,
    UP_TREND,
    DOWN_TREND,
    CRASH,
    PULL_BACK,
    SURGE
};

/**
 * Parameter struct for setting market events
 */
struct EventParams {
    Volatility volatility;
    double bias;
    double drift;
    double spread;
    uint32_t event_duration; // counter of how many cycles the event should last for
};

/**
 * @class MarketState
 * @brief simulated market microstructure with event-driven volatility regimes
 * maintains synthetic market state (price, volatility, bias) that drives order
 * generation patterns in benchmarks
 * supports multiple volatility regimes
 */
class MarketState {
public:
    explicit MarketState(std::optional<uint32_t> seed = std::nullopt);
    
    // getter methods
    double getPrice() const noexcept;
    double getVolatility() const noexcept;
    double getBias() const noexcept;
    double getDrift() const noexcept;
    double getSpread() const noexcept;
    MarketEvent getEvent() const noexcept;
    
    // events
    void applyEvent(MarketEvent evt, uint64_t current_cycle) noexcept;
    void setParameters(EventParams params) noexcept;
    void checkAndApplyEvent(uint64_t cycle_count) noexcept;
    void updatePrice() noexcept;
    double sampleVolatility(Volatility vol) noexcept;
    const EventParams& getEventParams(MarketEvent evt) const noexcept;

private:
    // base matrics
    std::atomic<double> base_price_{100.0};
    std::atomic<double> volatility_{0.005}; // ~0.5% per tick default
    std::atomic<double> bias_{0.50}; // 0.5 = balanced; >0.5 favors buys, <0.5 favors sells
    std::atomic<double> drift_{0.0}; // per-tick drift in price units
    std::atomic<double> spread_{0.05}; // half-spread in price units
    std::atomic<Volatility> curr_volatility_{Volatility::LOW};
    std::atomic<MarketEvent> event_{MarketEvent::BASE};

    // constexpr lookup table for different events
    static constexpr std::array<std::pair<MarketEvent, EventParams>, 6> event_params{
        // ------------------------------------------- volatility -------- bias - drift - spread - duration
        std::pair{MarketEvent::BASE,       EventParams{Volatility::LOW,    0.50,  0.000,  0.05,    0}},
        std::pair{MarketEvent::UP_TREND,   EventParams{Volatility::LOW,    0.60,  0.020,  0.05,  500}},
        std::pair{MarketEvent::DOWN_TREND, EventParams{Volatility::LOW,    0.40, -0.020,  0.05,  500}},
        std::pair{MarketEvent::SURGE,      EventParams{Volatility::MEDIUM, 0.72,  0.060,  0.08,  300}},
        std::pair{MarketEvent::PULL_BACK,  EventParams{Volatility::MEDIUM, 0.28, -0.060,  0.08,  300}},
        std::pair{MarketEvent::CRASH,      EventParams{Volatility::HIGH,   0.15, -0.120,  0.20,  200}},
    };
    
    // state for event generation
    std::mt19937 rng_;
    uint64_t cycle_count_{0};
    uint64_t last_event_cycle_{0};
    std::atomic<uint64_t> event_duration_{0};
    std::atomic<uint64_t> event_end_{0};
    
    // volatility distributions (values in basis points, divided by 10000 to get fraction)
    std::uniform_int_distribution<int> low_vol_dist_{30, 80};    // 0.30 - 0.80% per tick
    std::uniform_int_distribution<int> med_vol_dist_{80, 200};   // 0.80 - 2.00% per tick
    std::uniform_int_distribution<int> high_vol_dist_{200, 500}; // 2.00 - 5.00% per tick

};
