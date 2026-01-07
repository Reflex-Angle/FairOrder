#include "simulation/Trader.h"
#include <chrono>
#include <algorithm>

TraderSimulator::TraderSimulator() 
    : rng_(std::chrono::steady_clock::now().time_since_epoch().count()),
      price_spread_(-5, 5),
      qty_variation_(5, 15),
      side_choice_(0, 1) {}

std::vector<Trader> TraderSimulator::create_standard_traders() {
    std::vector<Trader> traders;
    
    // Fast trader (colocated): 5 microseconds
    traders.emplace_back(1, "FastTrader (5µs)", 5'000);
    
    // Medium trader: 25 microseconds
    traders.emplace_back(2, "MediumTrader (25µs)", 25'000);
    
    // Slow trader (remote): 50 microseconds
    traders.emplace_back(3, "SlowTrader (50µs)", 50'000);
    
    // Very slow trader: 100 microseconds
    traders.emplace_back(4, "VerySlowTrader (100µs)", 100'000);
    
    return traders;
}

TraderSimulator::OrderParams TraderSimulator::generate_order(
    const Trader& trader, Price center_price, Qty base_qty) {
    
    int price_offset = price_spread_(rng_);
    Price order_price = center_price + price_offset;
    
    int qty_offset = qty_variation_(rng_);
    Qty order_qty = base_qty + qty_offset;
    
    Side order_side = (side_choice_(rng_) == 0) ? Side::BUY : Side::SELL;
    
    return {order_price, order_qty, order_side};
}

