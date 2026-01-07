#pragma once

#include <map>
#include <vector>
#include <queue>
#include "core/OrderEvent.h"
#include "core/MatchingMode.h"

struct Order {
    OrderID order_id;
    Price price;
    Qty qty;
    Qty remaining_qty;
    TimeNs recv_time;
    BatchID batch_id;
    int trader_id;
    
    Order(const OrderEvent& ev, int trader_id)
        : order_id(ev.order_id), price(ev.price), qty(ev.qty),
          remaining_qty(ev.qty), recv_time(ev.recv_time),
          batch_id(ev.batch_id), trader_id(ev.trader_id != 0 ? ev.trader_id : trader_id) {}
};

struct Trade {
    OrderID buy_order_id;
    OrderID sell_order_id;
    Price price;
    Qty qty;
    TimeNs execution_time;
    int buy_trader_id;
    int sell_trader_id;
};

// Price-time priority queue for BUY orders (best price first, then earliest time)
struct BuyOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price < b.price; // Lower price = lower priority
        return a.recv_time > b.recv_time; // Later time = lower priority
    }
};

// Price-time priority queue for SELL orders (best price first, then earliest time)
struct SellOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price > b.price; // Higher price = lower priority
        return a.recv_time > b.recv_time; // Later time = lower priority
    }
};

// Fair priority: price first, then order_id (ignores recv_time)
struct FairBuyOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price < b.price;
        return a.order_id > b.order_id; // Higher ID = lower priority
    }
};

struct FairSellOrderComparator {
    bool operator()(const Order& a, const Order& b) const {
        if (a.price != b.price) return a.price > b.price;
        return a.order_id > b.order_id; // Higher ID = lower priority
    }
};

class OrderBook {
public:
    explicit OrderBook(MatchingMode mode);
    
    // Process a single order (naive mode) or batch (fair mode)
    std::vector<Trade> process_order(const OrderEvent& ev, int trader_id);
    std::vector<Trade> process_batch(const std::vector<OrderEvent>& batch, const std::vector<int>& trader_ids);
    
    // Get current best bid/ask
    Price get_best_bid() const;
    Price get_best_ask() const;
    
    // Get order book depth
    size_t get_buy_depth() const;
    size_t get_sell_depth() const;
    
    void clear();

private:
    MatchingMode mode_;
    
    // Naive mode: use priority queues with recv_time
    std::priority_queue<Order, std::vector<Order>, BuyOrderComparator> buy_orders_naive_;
    std::priority_queue<Order, std::vector<Order>, SellOrderComparator> sell_orders_naive_;
    
    // Fair mode: use priority queues with order_id
    std::priority_queue<Order, std::vector<Order>, FairBuyOrderComparator> buy_orders_fair_;
    std::priority_queue<Order, std::vector<Order>, FairSellOrderComparator> sell_orders_fair_;
    
    std::vector<Trade> match_order(Order& order, bool is_buy);
    static TimeNs get_current_time();
};

