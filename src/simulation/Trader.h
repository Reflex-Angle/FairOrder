#pragma once

#include <random>
#include <string>
#include "core/Types.h"
#include "core/OrderEvent.h"

struct Trader {
    int id;
    std::string name;
    TimeNs artificial_latency_ns;  // Simulated network latency
    int orders_submitted;
    int orders_executed;
    int trades_won;  // Number of times this trader's order executed first in a collision
    
    Trader(int id, const std::string& name, TimeNs latency_ns)
        : id(id), name(name), artificial_latency_ns(latency_ns),
          orders_submitted(0), orders_executed(0), trades_won(0) {}
    
    // Apply artificial latency to a timestamp
    TimeNs apply_latency(TimeNs base_time) const {
        return base_time + artificial_latency_ns;
    }
    
    void reset_stats() {
        orders_submitted = 0;
        orders_executed = 0;
        trades_won = 0;
    }
};

class TraderSimulator {
public:
    TraderSimulator();
    
    // Create predefined trader configurations
    std::vector<Trader> create_standard_traders();
    
    // Generate random order from a trader
    struct OrderParams {
        Price price;
        Qty qty;
        Side side;
    };
    
    OrderParams generate_order(const Trader& trader, Price center_price, Qty base_qty);
    
private:
    std::mt19937 rng_;
    std::uniform_int_distribution<int> price_spread_;
    std::uniform_int_distribution<int> qty_variation_;
    std::uniform_int_distribution<int> side_choice_;
};

