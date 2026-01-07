#include "book/OrderBook.h"
#include <chrono>
#include <algorithm>
#include <iostream>

OrderBook::OrderBook(MatchingMode mode) : mode_(mode) {}

Price OrderBook::get_best_bid() const {
    if (mode_ == MatchingMode::NAIVE_PRICE_TIME) {
        if (buy_orders_naive_.empty()) return 0;
        return buy_orders_naive_.top().price;
    } else {
        if (buy_orders_fair_.empty()) return 0;
        return buy_orders_fair_.top().price;
    }
}

Price OrderBook::get_best_ask() const {
    if (mode_ == MatchingMode::NAIVE_PRICE_TIME) {
        if (sell_orders_naive_.empty()) return 0;
        return sell_orders_naive_.top().price;
    } else {
        if (sell_orders_fair_.empty()) return 0;
        return sell_orders_fair_.top().price;
    }
}

size_t OrderBook::get_buy_depth() const {
    return mode_ == MatchingMode::NAIVE_PRICE_TIME 
        ? buy_orders_naive_.size() 
        : buy_orders_fair_.size();
}

size_t OrderBook::get_sell_depth() const {
    return mode_ == MatchingMode::NAIVE_PRICE_TIME 
        ? sell_orders_naive_.size() 
        : sell_orders_fair_.size();
}

void OrderBook::clear() {
    while (!buy_orders_naive_.empty()) buy_orders_naive_.pop();
    while (!sell_orders_naive_.empty()) sell_orders_naive_.pop();
    while (!buy_orders_fair_.empty()) buy_orders_fair_.pop();
    while (!sell_orders_fair_.empty()) sell_orders_fair_.pop();
}

TimeNs OrderBook::get_current_time() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

std::vector<Trade> OrderBook::process_order(const OrderEvent& ev, int trader_id) {
    Order order(ev, trader_id);
    return match_order(order, ev.side == Side::BUY);
}

std::vector<Trade> OrderBook::process_batch(const std::vector<OrderEvent>& batch, const std::vector<int>& trader_ids) {
    std::vector<Trade> all_trades;
    
    // Sort batch by priority before processing
    std::vector<std::pair<OrderEvent, int>> orders_with_traders;
    for (size_t i = 0; i < batch.size(); ++i) {
        orders_with_traders.push_back({batch[i], trader_ids[i]});
    }
    
    // Sort based on mode
    if (mode_ == MatchingMode::LATENCY_FAIR_BATCHED) {
        // Fair mode: price first, then order_id
        std::sort(orders_with_traders.begin(), orders_with_traders.end(),
            [](const auto& a, const auto& b) {
                const auto& ev_a = a.first;
                const auto& ev_b = b.first;
                
                if (ev_a.side != ev_b.side) return ev_a.side == Side::BUY;
                
                if (ev_a.side == Side::BUY) {
                    if (ev_a.price != ev_b.price) return ev_a.price > ev_b.price;
                    return ev_a.order_id < ev_b.order_id;
                } else {
                    if (ev_a.price != ev_b.price) return ev_a.price < ev_b.price;
                    return ev_a.order_id < ev_b.order_id;
                }
            });
    } else {
        // Naive mode: price first, then recv_time
        std::sort(orders_with_traders.begin(), orders_with_traders.end(),
            [](const auto& a, const auto& b) {
                const auto& ev_a = a.first;
                const auto& ev_b = b.first;
                
                if (ev_a.side != ev_b.side) return ev_a.side == Side::BUY;
                
                if (ev_a.side == Side::BUY) {
                    if (ev_a.price != ev_b.price) return ev_a.price > ev_b.price;
                    return ev_a.recv_time < ev_b.recv_time;
                } else {
                    if (ev_a.price != ev_b.price) return ev_a.price < ev_b.price;
                    return ev_a.recv_time < ev_b.recv_time;
                }
            });
    }
    
    // Process sorted orders
    for (const auto& [ev, trader_id] : orders_with_traders) {
        Order order(ev, trader_id);
        auto trades = match_order(order, ev.side == Side::BUY);
        all_trades.insert(all_trades.end(), trades.begin(), trades.end());
    }
    
    return all_trades;
}

std::vector<Trade> OrderBook::match_order(Order& order, bool is_buy) {
    std::vector<Trade> trades;
    TimeNs exec_time = get_current_time();
    
    if (mode_ == MatchingMode::NAIVE_PRICE_TIME) {
        if (is_buy) {
            // Match against sell orders
            while (order.remaining_qty > 0 && !sell_orders_naive_.empty()) {
                auto best_sell = sell_orders_naive_.top();
                if (best_sell.price > order.price) break; // No match possible
                
                sell_orders_naive_.pop();
                
                Qty trade_qty = std::min(order.remaining_qty, best_sell.remaining_qty);
                Price trade_price = best_sell.price; // Price-time priority: take maker's price
                
                trades.push_back({order.order_id, best_sell.order_id, trade_price, trade_qty, exec_time,
                                 order.trader_id, best_sell.trader_id});
                
                order.remaining_qty -= trade_qty;
                best_sell.remaining_qty -= trade_qty;
                
                if (best_sell.remaining_qty > 0) {
                    sell_orders_naive_.push(best_sell);
                }
            }
            
            // Add remaining quantity to book
            if (order.remaining_qty > 0) {
                buy_orders_naive_.push(order);
            }
        } else {
            // Match against buy orders
            while (order.remaining_qty > 0 && !buy_orders_naive_.empty()) {
                auto best_buy = buy_orders_naive_.top();
                if (best_buy.price < order.price) break; // No match possible
                
                buy_orders_naive_.pop();
                
                Qty trade_qty = std::min(order.remaining_qty, best_buy.remaining_qty);
                Price trade_price = best_buy.price; // Price-time priority: take maker's price
                
                trades.push_back({best_buy.order_id, order.order_id, trade_price, trade_qty, exec_time,
                                 best_buy.trader_id, order.trader_id});
                
                order.remaining_qty -= trade_qty;
                best_buy.remaining_qty -= trade_qty;
                
                if (best_buy.remaining_qty > 0) {
                    buy_orders_naive_.push(best_buy);
                }
            }
            
            // Add remaining quantity to book
            if (order.remaining_qty > 0) {
                sell_orders_naive_.push(order);
            }
        }
    } else {
        // Fair mode
        if (is_buy) {
            while (order.remaining_qty > 0 && !sell_orders_fair_.empty()) {
                auto best_sell = sell_orders_fair_.top();
                if (best_sell.price > order.price) break;
                
                sell_orders_fair_.pop();
                
                Qty trade_qty = std::min(order.remaining_qty, best_sell.remaining_qty);
                Price trade_price = best_sell.price;
                
                trades.push_back({order.order_id, best_sell.order_id, trade_price, trade_qty, exec_time,
                                 order.trader_id, best_sell.trader_id});
                
                order.remaining_qty -= trade_qty;
                best_sell.remaining_qty -= trade_qty;
                
                if (best_sell.remaining_qty > 0) {
                    sell_orders_fair_.push(best_sell);
                }
            }
            
            if (order.remaining_qty > 0) {
                buy_orders_fair_.push(order);
            }
        } else {
            while (order.remaining_qty > 0 && !buy_orders_fair_.empty()) {
                auto best_buy = buy_orders_fair_.top();
                if (best_buy.price < order.price) break;
                
                buy_orders_fair_.pop();
                
                Qty trade_qty = std::min(order.remaining_qty, best_buy.remaining_qty);
                Price trade_price = best_buy.price;
                
                trades.push_back({best_buy.order_id, order.order_id, trade_price, trade_qty, exec_time,
                                 best_buy.trader_id, order.trader_id});
                
                order.remaining_qty -= trade_qty;
                best_buy.remaining_qty -= trade_qty;
                
                if (best_buy.remaining_qty > 0) {
                    buy_orders_fair_.push(best_buy);
                }
            }
            
            if (order.remaining_qty > 0) {
                sell_orders_fair_.push(order);
            }
        }
    }
    
    return trades;
}

