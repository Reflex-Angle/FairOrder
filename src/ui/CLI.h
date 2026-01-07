#pragma once

#include <string>
#include <vector>
#include "core/MatchingMode.h"
#include "simulation/Trader.h"
#include "engine/MatchingEngine.h"
#include "batching/MicroBatcher.h"

class CLI {
public:
    CLI();
    void run();

private:
    MatchingMode current_mode_;
    TimeNs batch_window_ns_;
    MatchingEngine* engine_;
    MicroBatcher* batcher_;
    std::vector<Trader> traders_;
    TraderSimulator simulator_;
    
    void print_banner();
    void print_help();
    void print_menu();
    
    void run_simulation(int num_orders);
    void run_experiment();
    void compare_modes(int num_orders);
    
    void set_mode(const std::string& mode_str);
    void set_batch_window(const std::string& window_str);
    void show_metrics();
    void show_order_book();
    void reset();
    
    std::string mode_to_string(MatchingMode mode) const;
    MatchingMode string_to_mode(const std::string& str) const;
    
    // Helper to parse time strings like "100us", "1ms"
    TimeNs parse_time_string(const std::string& str);
};

