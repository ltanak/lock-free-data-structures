#include "order_simulation/market_state.hpp"
#include <random>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>

MarketState::MarketState(std::optional<uint32_t> seed) {
    if (seed.has_value()) {
        rng_ = std::mt19937(seed.value());
    } else {
        std::random_device rd;
        rng_ = std::mt19937(rd());
    }
}

double MarketState::getPrice() const noexcept { return base_price_.load(std::memory_order_relaxed); }
double MarketState::getVolatility() const noexcept { return volatility_.load(std::memory_order_relaxed); }
double MarketState::getBias() const noexcept { return bias_.load(std::memory_order_relaxed); }
double MarketState::getDrift() const noexcept { return drift_.load(std::memory_order_relaxed); }
double MarketState::getSpread() const noexcept { return spread_.load(std::memory_order_relaxed); }
MarketEvent MarketState::getEvent() const noexcept { return event_.load(std::memory_order_relaxed); }

/**
 * sample volatility as decimals, representing percentages
 */

double MarketState::sampleVolatility(Volatility vol) noexcept {
    switch(vol){
        case Volatility::LOW:
            return low_vol_dist_(rng_) / 100.0;
        case Volatility::MEDIUM:
            return med_vol_dist_(rng_) / 100.0;
        case Volatility::HIGH:
            return high_vol_dist_(rng_) / 100.0;
        default:
            return 0.15;
    }
}

const EventParams& MarketState::getEventParams(MarketEvent evt) const noexcept {
    for (const auto& [event, params]: event_params) {
        if (event == evt) return params;
    }
    // Fallback to BASE
    return event_params[0].second;
}

void MarketState::updatePrice() noexcept {
    double current_price = base_price_.load(std::memory_order_relaxed);
    double drift = drift_.load(std::memory_order_relaxed);
    double vol = volatility_.load(std::memory_order_relaxed);
    
    std::normal_distribution<double> price_noise{0.0, vol};
    double price_change = drift + price_noise(rng_);
    
    double new_price = std::clamp(current_price + price_change, 85.0, 115.0);
    base_price_.store(new_price, std::memory_order_relaxed);
}

void MarketState::applyEvent(MarketEvent evt, uint64_t current_cycle) noexcept {
    const auto& params = getEventParams(evt);
    
    // sample volatility within regime
    double sampled_vol = sampleVolatility(params.volatility);
    
    // stochastic noise to base parameters (reduced noise)
    std::normal_distribution<double> drift_noise{0.0, 0.00005};
    std::normal_distribution<double> bias_noise{0.0, 0.03};
    
    double final_drift = params.drift + drift_noise(rng_);
    double final_bias = std::clamp(params.bias + bias_noise(rng_), 0.0, 1.0);
    
    volatility_.store(sampled_vol, std::memory_order_relaxed);
    drift_.store(final_drift, std::memory_order_relaxed);
    bias_.store(final_bias, std::memory_order_relaxed);
    spread_.store(params.spread, std::memory_order_relaxed);
    curr_volatility_.store(params.volatility, std::memory_order_relaxed);
    event_.store(evt, std::memory_order_relaxed);
    
    event_end_.store(current_cycle + params.event_duration, std::memory_order_relaxed);
}

void MarketState::checkAndApplyEvent(uint64_t cycle_count) noexcept {
    // check if current event expired
    uint64_t end_cycle = event_end_.load(std::memory_order_relaxed);
    if (cycle_count > 0 && end_cycle > 0 && cycle_count >= end_cycle) {
        // Event expired; trigger new one every N cycles or return to BASE
        if ((cycle_count - last_event_cycle_) % 1000 == 0) {
            // Randomly pick next event
            std::uniform_int_distribution<int> event_dist{0, 5};
            int choice = event_dist(rng_);
            
            MarketEvent next_event;
            switch(choice) {
                case 1: next_event = MarketEvent::UP_TREND; break;
                case 2: next_event = MarketEvent::DOWN_TREND; break;
                case 3: next_event = MarketEvent::CRASH; break;
                case 4: next_event = MarketEvent::SURGE; break;
                case 5: next_event = MarketEvent::PULL_BACK; break;
                default: next_event = MarketEvent::BASE;
            }
            
            applyEvent(next_event, cycle_count);
            last_event_cycle_ = cycle_count;
        }
    }
}

void MarketState::applyEvent(MarketEvent event_type) noexcept {
    // Legacy override - just call the new version with cycle 0
    applyEvent(event_type, 0);
}

void MarketState::checkEvent(uint64_t cycle_count) noexcept {
    // Legacy method for compatibility
    checkAndApplyEvent(cycle_count);
}

void MarketState::setParameters(EventParams params) noexcept {
    volatility_.store(params.drift, std::memory_order_relaxed);
    drift_.store(params.drift, std::memory_order_relaxed);
    bias_.store(params.bias, std::memory_order_relaxed);
    spread_.store(params.spread, std::memory_order_relaxed);
}
