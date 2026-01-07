#pragma once

#include <map>
#include <vector>
#include <string>
#include "simulation/Trader.h"
#include "book/OrderBook.h"

struct TradeRecord {
    int buy_trader_id;
    int sell_trader_id;
    Price price;
    Qty qty;
    TimeNs execution_time;
    bool was_collision;  // True if multiple traders submitted same-price orders
};

struct TraderStats {
    int trader_id;
    std::string name;
    int orders_submitted;
    int orders_executed;
    int trades_won;
    int trades_lost;
    double win_rate;
    double execution_rate;
    TimeNs total_latency_ns;
};

class FairnessMetrics {
public:
    FairnessMetrics();
    
    void record_trade(const Trade& trade, bool was_collision = false);
    void record_order_submission(int trader_id);
    void record_order_execution(int trader_id);
    void record_trade_win(int trader_id);
    void record_trade_loss(int trader_id);
    
    // Compute fairness metrics
    double compute_fairness_index() const;
    double compute_latency_advantage_reduction(const std::vector<Trader>& traders) const;
    
    // Generate statistics
    std::vector<TraderStats> get_trader_stats(const std::vector<Trader>& traders) const;
    
    // Reset all metrics
    void reset();
    
    // Get summary string
    std::string get_summary(const std::vector<Trader>& traders) const;
    
    // Get detailed report
    std::string get_detailed_report(const std::vector<Trader>& traders) const;

private:
    std::map<int, int> trader_orders_submitted_;
    std::map<int, int> trader_orders_executed_;
    std::map<int, int> trader_trades_won_;
    std::map<int, int> trader_trades_lost_;
    std::vector<TradeRecord> trade_history_;
    
    double compute_win_rate_imbalance() const;
};

