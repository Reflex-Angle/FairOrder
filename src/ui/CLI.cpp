#include "ui/CLI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <random>
#include "core/OrderEvent.h"

static TimeNs now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

CLI::CLI() 
    : current_mode_(MatchingMode::LATENCY_FAIR_BATCHED),
      batch_window_ns_(100'000),  // 100 microseconds
      engine_(nullptr),
      batcher_(nullptr),
      simulator_() {
    traders_ = simulator_.create_standard_traders();
    engine_ = new MatchingEngine(current_mode_);
    batcher_ = new MicroBatcher(batch_window_ns_);
}

void CLI::print_banner() {
    std::cout << R"(
====================================================================
                                                                  
          FairOrder: Latency-Fair Matching Engine Demo            
                                                                  
     Demonstrating how micro-batching reduces latency arbitrage   
                                                                  
====================================================================
)";
}

void CLI::print_help() {
    std::cout << R"(
Available Commands:
  help              - Show this help message
  menu              - Show main menu
  mode <naive|fair> - Set matching mode (naive = price-time, fair = batched)
  window <time>     - Set batch window (e.g., 100us, 1ms)
  simulate <N>      - Run simulation with N orders
  experiment        - Run comparative experiment (naive vs fair)
  metrics           - Show current fairness metrics
  book              - Show order book state
  reset             - Reset engine and metrics
  quit/exit         - Exit the program

Example usage:
  mode fair
  window 50us
  simulate 1000
  experiment
)";
}

void CLI::print_menu() {
    std::cout << "\n====================================================================\n";
    std::cout << "                         MAIN MENU                                \n";
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << " Current Mode: " << std::setw(20) << std::left << mode_to_string(current_mode_) << "                    \n";
    std::cout << " Batch Window: " << std::setw(20) << std::left << (std::to_string(batch_window_ns_ / 1000) + "us") << "                    \n";
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << " 1. Run Simulation (1000 orders)                                \n";
    std::cout << " 2. Run Comparative Experiment (Naive vs Fair)                   \n";
    std::cout << " 3. Configure Mode (naive/fair)                                   \n";
    std::cout << " 4. Configure Batch Window                                       \n";
    std::cout << " 5. Show Metrics                                                 \n";
    std::cout << " 6. Show Order Book                                              \n";
    std::cout << " 7. Reset                                                        \n";
    std::cout << " 8. Help                                                         \n";
    std::cout << " 9. Exit                                                         \n";
    std::cout << "====================================================================\n";
    std::cout << "\nEnter command or menu option: ";
}

void CLI::run() {
    print_banner();
    print_help();
    
    std::string input;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;
        
        if (cmd == "quit" || cmd == "exit" || cmd == "9") {
            std::cout << "Exiting...\n";
            break;
        } else if (cmd == "help" || cmd == "8") {
            print_help();
        } else if (cmd == "menu") {
            print_menu();
        } else if (cmd == "mode" || cmd == "3") {
            std::string mode_str;
            if (cmd == "3") {
                std::cout << "Enter mode (naive/fair): ";
                std::getline(std::cin, mode_str);
            } else {
                iss >> mode_str;
            }
            set_mode(mode_str);
        } else if (cmd == "window" || cmd == "4") {
            std::string window_str;
            if (cmd == "4") {
                std::cout << "Enter batch window (e.g., 100us): ";
                std::getline(std::cin, window_str);
            } else {
                iss >> window_str;
            }
            set_batch_window(window_str);
        } else if (cmd == "simulate" || cmd == "1") {
            int num_orders = 1000;
            if (cmd == "1") {
                std::cout << "Enter number of orders (default 1000): ";
                std::string num_str;
                std::getline(std::cin, num_str);
                if (!num_str.empty()) {
                    num_orders = std::stoi(num_str);
                }
            } else {
                iss >> num_orders;
            }
            run_simulation(num_orders);
        } else if (cmd == "experiment" || cmd == "2") {
            run_experiment();
        } else if (cmd == "metrics" || cmd == "5") {
            show_metrics();
        } else if (cmd == "book" || cmd == "6") {
            show_order_book();
        } else if (cmd == "reset" || cmd == "7") {
            reset();
        } else {
            std::cout << "Unknown command. Type 'help' for available commands.\n";
        }
    }
    
    delete engine_;
    delete batcher_;
}

void CLI::run_simulation(int num_orders) {
    std::cout << "\n====================================================================\n";
    std::cout << " Running Simulation: " << std::setw(40) << std::left << (std::to_string(num_orders) + " orders") << " \n";
    std::cout << " Mode: " << std::setw(52) << std::left << mode_to_string(current_mode_) << " \n";
    std::cout << "====================================================================\n";
    
    reset();
    
    Price center_price = 100;
    Qty base_qty = 10;
    OrderID next_order_id = 1;
    
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<size_t> trader_dist(0, traders_.size() - 1);
    
    std::cout << "\nGenerating and processing orders...\n";
    
    for (int i = 0; i < num_orders; ++i) {
        // Pick random trader
        size_t trader_idx = trader_dist(rng);
        const auto& trader = traders_[trader_idx];
        
        // Generate order
        auto params = simulator_.generate_order(trader, center_price, base_qty);
        
        // Create order event
        TimeNs base_time = now_ns();
        TimeNs recv_time = trader.apply_latency(base_time);
        
        OrderEvent ev;
        ev.type = EventType::NEW;
        ev.order_id = next_order_id++;
        ev.instrument = "STOCK";
        ev.side = params.side;
        ev.price = params.price;
        ev.qty = params.qty;
        ev.recv_time = recv_time;
        ev.trader_id = trader.id;
        
        // Process based on mode
        if (current_mode_ == MatchingMode::LATENCY_FAIR_BATCHED) {
            batcher_->submit(std::move(ev));
            
            // Check for ready batches
            while (batcher_->has_ready_batch()) {
                auto batch = batcher_->pop_batch();
                std::vector<int> trader_ids;
                trader_ids.reserve(batch.size());
                for (const auto& order : batch) {
                    trader_ids.push_back(order.trader_id);
                }
                engine_->process_batch(batch, trader_ids);
            }
        } else {
            // Naive mode: also batch orders to detect competitions, but process in recv_time order
            batcher_->submit(std::move(ev));
            
            // Check for ready batches
            while (batcher_->has_ready_batch()) {
                auto batch = batcher_->pop_batch();
                // Sort by recv_time for naive mode (price-time priority)
                std::vector<std::pair<OrderEvent, int>> orders_with_traders;
                for (const auto& order : batch) {
                    orders_with_traders.push_back({order, order.trader_id});
                }
                
                // Sort by price (best first), then recv_time
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
                
                // Extract sorted batch and trader IDs
                std::vector<OrderEvent> sorted_batch;
                std::vector<int> trader_ids;
                sorted_batch.reserve(orders_with_traders.size());
                trader_ids.reserve(orders_with_traders.size());
                
                for (const auto& [order, trader_id] : orders_with_traders) {
                    sorted_batch.push_back(order);
                    trader_ids.push_back(trader_id);
                }
                
                engine_->process_batch(sorted_batch, trader_ids);
            }
        }
        
        // Small delay to simulate real-time arrival
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        
        if ((i + 1) % 100 == 0) {
            std::cout << "Processed " << (i + 1) << " orders...\r" << std::flush;
        }
    }
    
    // Flush any remaining batch
    if (!batcher_->has_ready_batch()) {
        // Force flush
        auto batch = batcher_->pop_batch();
        if (!batch.empty()) {
            if (current_mode_ == MatchingMode::LATENCY_FAIR_BATCHED) {
                std::vector<int> trader_ids;
                trader_ids.reserve(batch.size());
                for (const auto& order : batch) {
                    trader_ids.push_back(order.trader_id);
                }
                engine_->process_batch(batch, trader_ids);
            } else {
                // Naive mode: sort by recv_time
                std::vector<std::pair<OrderEvent, int>> orders_with_traders;
                for (const auto& order : batch) {
                    orders_with_traders.push_back({order, order.trader_id});
                }
                
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
                
                std::vector<OrderEvent> sorted_batch;
                std::vector<int> trader_ids;
                sorted_batch.reserve(orders_with_traders.size());
                trader_ids.reserve(orders_with_traders.size());
                
                for (const auto& [order, trader_id] : orders_with_traders) {
                    sorted_batch.push_back(order);
                    trader_ids.push_back(trader_id);
                }
                
                engine_->process_batch(sorted_batch, trader_ids);
            }
        }
    }
    
    std::cout << "\n\nSimulation complete!\n";
    show_metrics();
}

void CLI::run_experiment() {
    std::cout << "\n====================================================================\n";
    std::cout << "              COMPARATIVE EXPERIMENT: Naive vs Fair                \n";
    std::cout << "====================================================================\n";
    
    int num_orders = 1000;
    std::cout << "\nRunning experiment with " << num_orders << " orders per mode...\n\n";
    
    // Test NAIVE mode
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << "MODE: NAIVE (Price-Time Priority)\n";
    std::cout << "--------------------------------------------------------------------\n";
    current_mode_ = MatchingMode::NAIVE_PRICE_TIME;
    delete engine_;
    engine_ = new MatchingEngine(current_mode_);
    run_simulation(num_orders);
    auto naive_metrics = engine_->get_metrics();
    auto naive_stats = naive_metrics.get_trader_stats(traders_);
    
    std::cout << "\n\n";
    
    // Test FAIR mode
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << "MODE: FAIR (Latency-Fair Batched)\n";
    std::cout << "--------------------------------------------------------------------\n";
    current_mode_ = MatchingMode::LATENCY_FAIR_BATCHED;
    delete engine_;
    engine_ = new MatchingEngine(current_mode_);
    delete batcher_;
    batcher_ = new MicroBatcher(batch_window_ns_);
    run_simulation(num_orders);
    auto fair_metrics = engine_->get_metrics();
    auto fair_stats = fair_metrics.get_trader_stats(traders_);
    
    // Comparison
    std::cout << "\n\n";
    std::cout << "====================================================================\n";
    std::cout << "                    EXPERIMENT RESULTS                            \n";
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << " Metric                    | Naive Mode | Fair Mode | Improvement\n";
    std::cout << "--------------------------------------------------------------------\n";
    
    double naive_fairness = naive_metrics.compute_fairness_index();
    double fair_fairness = fair_metrics.compute_fairness_index();
    double improvement = ((fair_fairness - naive_fairness) / (1.0 - naive_fairness)) * 100.0;
    
    std::cout << " Fairness Index            | " << std::setw(10) << std::fixed << std::setprecision(3) << naive_fairness
              << " | " << std::setw(9) << fair_fairness << " | " << std::setw(10) << improvement << "%\n";
    
    double naive_reduction = naive_metrics.compute_latency_advantage_reduction(traders_);
    double fair_reduction = fair_metrics.compute_latency_advantage_reduction(traders_);
    
    std::cout << " Latency Advantage         | " << std::setw(10) << (naive_reduction * 100)
              << " | " << std::setw(9) << (fair_reduction * 100) << " | " << std::setw(10) 
              << ((fair_reduction - naive_reduction) * 100) << "%\n";
    std::cout << " Reduction                 |            |           |            \n";
    
    // Show trader win rates
    std::cout << "--------------------------------------------------------------------\n";
    std::cout << " Trader Win Rates:                                                \n";
    
    for (size_t i = 0; i < traders_.size() && i < naive_stats.size() && i < fair_stats.size(); ++i) {
        std::cout << " " << std::setw(20) << std::left << traders_[i].name.substr(0, 20)
                  << " | " << std::setw(10) << (naive_stats[i].win_rate * 100) << "%"
                  << " | " << std::setw(9) << (fair_stats[i].win_rate * 100) << "%"
                  << " | " << std::setw(10) << ((fair_stats[i].win_rate - naive_stats[i].win_rate) * 100) << "%\n";
    }
    
    std::cout << "====================================================================\n";
}

void CLI::set_mode(const std::string& mode_str) {
    MatchingMode new_mode = string_to_mode(mode_str);
    if (new_mode != current_mode_) {
        current_mode_ = new_mode;
        delete engine_;
        engine_ = new MatchingEngine(current_mode_);
        if (current_mode_ == MatchingMode::LATENCY_FAIR_BATCHED) {
            delete batcher_;
            batcher_ = new MicroBatcher(batch_window_ns_);
        }
        std::cout << "Mode set to: " << mode_to_string(current_mode_) << "\n";
    } else {
        std::cout << "Mode is already: " << mode_to_string(current_mode_) << "\n";
    }
}

void CLI::set_batch_window(const std::string& window_str) {
    TimeNs new_window = parse_time_string(window_str);
    if (new_window > 0) {
        batch_window_ns_ = new_window;
        delete batcher_;
        batcher_ = new MicroBatcher(batch_window_ns_);
        std::cout << "Batch window set to: " << (batch_window_ns_ / 1000) << "µs\n";
    } else {
        std::cout << "Invalid time format. Use format like '100us' or '1ms'\n";
    }
}

void CLI::show_metrics() {
    std::cout << engine_->get_metrics().get_summary(traders_);
    std::cout << engine_->get_metrics().get_detailed_report(traders_);
}

void CLI::show_order_book() {
    const auto& book = engine_->get_order_book();
    std::cout << "\n========================================\n";
    std::cout << "          ORDER BOOK STATE            \n";
    std::cout << "----------------------------------------\n";
    std::cout << " Best Bid: " << std::setw(27) << book.get_best_bid() << " \n";
    std::cout << " Best Ask: " << std::setw(27) << book.get_best_ask() << " \n";
    std::cout << " Buy Depth: " << std::setw(26) << book.get_buy_depth() << " \n";
    std::cout << " Sell Depth: " << std::setw(25) << book.get_sell_depth() << " \n";
    std::cout << "========================================\n";
}

void CLI::reset() {
    delete engine_;
    engine_ = new MatchingEngine(current_mode_);
    delete batcher_;
    batcher_ = new MicroBatcher(batch_window_ns_);
    engine_->get_metrics().reset();
    for (auto& trader : traders_) {
        trader.reset_stats();
    }
    std::cout << "Engine and metrics reset.\n";
}

std::string CLI::mode_to_string(MatchingMode mode) const {
    return (mode == MatchingMode::NAIVE_PRICE_TIME) ? "Naive (Price-Time)" : "Fair (Batched)";
}

MatchingMode CLI::string_to_mode(const std::string& str) const {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "naive" || lower == "price-time") {
        return MatchingMode::NAIVE_PRICE_TIME;
    } else if (lower == "fair" || lower == "batched") {
        return MatchingMode::LATENCY_FAIR_BATCHED;
    }
    return MatchingMode::LATENCY_FAIR_BATCHED; // default
}

TimeNs CLI::parse_time_string(const std::string& str) {
    std::string s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    
    size_t unit_pos = s.find_first_not_of("0123456789.");
    if (unit_pos == std::string::npos) return 0;
    
    double value = std::stod(s.substr(0, unit_pos));
    std::string unit = s.substr(unit_pos);
    
    if (unit == "ns") return static_cast<TimeNs>(value);
    if (unit == "us" || unit == "µs") return static_cast<TimeNs>(value * 1000);
    if (unit == "ms") return static_cast<TimeNs>(value * 1'000'000);
    if (unit == "s") return static_cast<TimeNs>(value * 1'000'000'000);
    
    return 0;
}

