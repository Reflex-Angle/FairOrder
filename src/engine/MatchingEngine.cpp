#include "engine/MatchingEngine.h"
#include <iostream>
#include <map>

MatchingEngine::MatchingEngine(MatchingMode mode)
    : mode_(mode), order_book_(mode) {}

void MatchingEngine::set_mode(MatchingMode mode) {
    mode_ = mode;
    order_book_ = OrderBook(mode);
    metrics_.reset();
}

void MatchingEngine::process_batch(const std::vector<OrderEvent>& batch, const std::vector<int>& trader_ids) {
    if (batch.empty()) return;

    // Group orders by price and side to find competitions
    std::map<std::pair<Price, Side>, std::vector<std::pair<size_t, int>>> competitions;
    for (size_t i = 0; i < batch.size(); ++i) {
        competitions[{batch[i].price, batch[i].side}].push_back({i, trader_ids[i]});
    }

    auto trades = order_book_.process_batch(batch, trader_ids);
    
    // Record all trades
    for (const auto& trade : trades) {
        metrics_.record_trade(trade, false);
    }
    
    // For each price/side combination with multiple traders, determine winner
    for (const auto& [price_side, competitors] : competitions) {
        if (competitors.size() < 2) continue; // No competition
        
        // Find which competitor executed first (based on priority)
        int winner_trader_id = -1;
        size_t winner_index = batch.size();
        
        for (const auto& [idx, trader_id] : competitors) {
            const auto& ev = batch[idx];
            bool is_better = false;
            
            if (winner_trader_id == -1) {
                is_better = true;
            } else {
                const auto& winner_ev = batch[winner_index];
                if (mode_ == MatchingMode::LATENCY_FAIR_BATCHED) {
                    // Fair mode: lower order_id wins
                    is_better = (ev.order_id < winner_ev.order_id);
                } else {
                    // Naive mode: earlier recv_time wins
                    is_better = (ev.recv_time < winner_ev.recv_time);
                }
            }
            
            if (is_better) {
                winner_trader_id = trader_id;
                winner_index = idx;
            }
        }
        
        // Record win for winner, losses for others
        if (winner_trader_id != -1) {
            metrics_.record_trade_win(winner_trader_id);
            for (const auto& [idx, trader_id] : competitors) {
                if (trader_id != winner_trader_id) {
                    metrics_.record_trade_loss(trader_id);
                }
            }
        }
    }
}

void MatchingEngine::process_order(const OrderEvent& ev, int trader_id) {
    metrics_.record_order_submission(trader_id);
    
    auto trades = order_book_.process_order(ev, trader_id);
    
    for (const auto& trade : trades) {
        metrics_.record_trade(trade, false);
    }
    
    // Note: In naive mode, we process orders immediately, so we can't easily detect
    // competitions within a batch. However, we can track competitions by looking
    // at orders that execute against the same price level in the book.
    // For now, we'll track this differently - by checking if multiple orders
    // at the same price executed around the same time.
}
