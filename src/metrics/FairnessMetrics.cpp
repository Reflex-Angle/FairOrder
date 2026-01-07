#include "metrics/FairnessMetrics.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>

FairnessMetrics::FairnessMetrics() {}

void FairnessMetrics::record_trade(const Trade& trade, bool was_collision) {
    trade_history_.push_back({
        trade.buy_trader_id,
        trade.sell_trader_id,
        trade.price,
        trade.qty,
        trade.execution_time,
        was_collision
    });
    
    record_order_execution(trade.buy_trader_id);
    record_order_execution(trade.sell_trader_id);
}

void FairnessMetrics::record_order_submission(int trader_id) {
    trader_orders_submitted_[trader_id]++;
}

void FairnessMetrics::record_order_execution(int trader_id) {
    trader_orders_executed_[trader_id]++;
}

void FairnessMetrics::record_trade_win(int trader_id) {
    trader_trades_won_[trader_id]++;
}

void FairnessMetrics::record_trade_loss(int trader_id) {
    trader_trades_lost_[trader_id]++;
}

void FairnessMetrics::reset() {
    trader_orders_submitted_.clear();
    trader_orders_executed_.clear();
    trader_trades_won_.clear();
    trader_trades_lost_.clear();
    trade_history_.clear();
}

double FairnessMetrics::compute_fairness_index() const {
    if (trader_trades_won_.empty()) return 0.0;
    
    // Find min and max win rates
    double min_win_rate = 1.0;
    double max_win_rate = 0.0;
    
    for (const auto& [trader_id, wins] : trader_trades_won_) {
        int lost = trader_trades_lost_.count(trader_id) > 0 ? trader_trades_lost_.at(trader_id) : 0;
        int total = wins + lost;
        if (total > 0) {
            double win_rate = static_cast<double>(wins) / total;
            min_win_rate = std::min(min_win_rate, win_rate);
            max_win_rate = std::max(max_win_rate, win_rate);
        }
    }
    
    // Fairness index: 1 - |max_win_rate - min_win_rate|
    return 1.0 - (max_win_rate - min_win_rate);
}

double FairnessMetrics::compute_latency_advantage_reduction(const std::vector<Trader>& traders) const {
    if (traders.size() < 2) return 0.0;
    
    // Find fastest and slowest traders
    auto fastest = std::min_element(traders.begin(), traders.end(),
        [](const Trader& a, const Trader& b) {
            return a.artificial_latency_ns < b.artificial_latency_ns;
        });
    auto slowest = std::max_element(traders.begin(), traders.end(),
        [](const Trader& a, const Trader& b) {
            return a.artificial_latency_ns < b.artificial_latency_ns;
        });
    
    int fast_wins = trader_trades_won_.count(fastest->id) > 0 ? trader_trades_won_.at(fastest->id) : 0;
    int slow_wins = trader_trades_won_.count(slowest->id) > 0 ? trader_trades_won_.at(slowest->id) : 0;
    int fast_lost = trader_trades_lost_.count(fastest->id) > 0 ? trader_trades_lost_.at(fastest->id) : 0;
    int slow_lost = trader_trades_lost_.count(slowest->id) > 0 ? trader_trades_lost_.at(slowest->id) : 0;
    int fast_total = fast_wins + fast_lost;
    int slow_total = slow_wins + slow_lost;
    
    if (fast_total == 0 || slow_total == 0) return 0.0;
    
    double fast_win_rate = static_cast<double>(fast_wins) / fast_total;
    double slow_win_rate = static_cast<double>(slow_wins) / slow_total;
    
    // In a perfectly fair system, both should have ~50% win rate
    // Reduction = how close we are to 50/50
    double ideal_rate = 0.5;
    double fast_deviation = std::abs(fast_win_rate - ideal_rate);
    double slow_deviation = std::abs(slow_win_rate - ideal_rate);
    
    // Reduction = 1 - average deviation from ideal
    return 1.0 - ((fast_deviation + slow_deviation) / 2.0);
}

std::vector<TraderStats> FairnessMetrics::get_trader_stats(const std::vector<Trader>& traders) const {
    std::vector<TraderStats> stats;
    
    for (const auto& trader : traders) {
        TraderStats s;
        s.trader_id = trader.id;
        s.name = trader.name;
        s.orders_submitted = trader_orders_submitted_.count(trader.id) > 0 ? trader_orders_submitted_.at(trader.id) : 0;
        s.orders_executed = trader_orders_executed_.count(trader.id) > 0 ? trader_orders_executed_.at(trader.id) : 0;
        s.trades_won = trader_trades_won_.count(trader.id) > 0 ? trader_trades_won_.at(trader.id) : 0;
        s.trades_lost = trader_trades_lost_.count(trader.id) > 0 ? trader_trades_lost_.at(trader.id) : 0;
        
        int total_trades = s.trades_won + s.trades_lost;
        s.win_rate = (total_trades > 0) ? static_cast<double>(s.trades_won) / total_trades : 0.0;
        s.execution_rate = (s.orders_submitted > 0) 
            ? static_cast<double>(s.orders_executed) / s.orders_submitted : 0.0;
        s.total_latency_ns = trader.artificial_latency_ns;
        
        stats.push_back(s);
    }
    
    return stats;
}

double FairnessMetrics::compute_win_rate_imbalance() const {
    if (trader_trades_won_.empty()) return 1.0;
    
    std::vector<double> win_rates;
    for (const auto& [trader_id, wins] : trader_trades_won_) {
        int lost = trader_trades_lost_.count(trader_id) > 0 ? trader_trades_lost_.at(trader_id) : 0;
        int total = wins + lost;
        if (total > 0) {
            win_rates.push_back(static_cast<double>(wins) / total);
        }
    }
    
    if (win_rates.empty()) return 1.0;
    
    double min_rate = *std::min_element(win_rates.begin(), win_rates.end());
    double max_rate = *std::max_element(win_rates.begin(), win_rates.end());
    
    return max_rate - min_rate;
}

std::string FairnessMetrics::get_summary(const std::vector<Trader>& traders) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    double fairness = compute_fairness_index();
    double reduction = compute_latency_advantage_reduction(traders);
    
    oss << "\n========================================\n";
    oss << "      FAIRNESS METRICS SUMMARY         \n";
    oss << "----------------------------------------\n";
    oss << " Fairness Index:      " << std::setw(6) << fairness << "        \n";
    oss << " Latency Advantage    " << std::setw(6) << (reduction * 100) << "%        \n";
    oss << " Reduction:                              \n";
    oss << "========================================\n";
    
    return oss.str();
}

std::string FairnessMetrics::get_detailed_report(const std::vector<Trader>& traders) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    auto stats = get_trader_stats(traders);
    
    oss << "\n====================================================================\n";
    oss << "                    DETAILED TRADER STATISTICS                   \n";
    oss << "--------------------------------------------------------------------\n";
    oss << " Trader              | Latency | Orders | Exec | Win Rate | Wins \n";
    oss << "--------------------------------------------------------------------\n";
    
    for (const auto& s : stats) {
        std::string latency_str = std::to_string(s.total_latency_ns / 1000) + "us";
        oss << " " << std::setw(19) << std::left << s.name.substr(0, 19)
            << " | " << std::setw(7) << latency_str
            << " | " << std::setw(6) << s.orders_submitted
            << " | " << std::setw(4) << s.orders_executed
            << " | " << std::setw(8) << (s.win_rate * 100) << "%"
            << " | " << std::setw(4) << s.trades_won << " \n";
    }
    
    oss << "====================================================================\n";
    
    return oss.str();
}

