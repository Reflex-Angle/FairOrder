#include <iostream>
#include <chrono>
#include <thread>

#include "batching/MicroBatcher.h"
#include "engine/MatchingEngine.h"

static TimeNs now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

int main() {

    /*
    std::cout << "Engine started.\n";
    MicroBatcher batcher(100'000); // 100 µs
    MatchingEngine engine;

    // Simulate incoming orders
    for (int i = 0; i < 10; i++) {
    OrderEvent ev;
    ev.type = EventType::NEW;
    ev.order_id = i + 1;
    ev.instrument = "ABC";
    ev.side = Side::BUY;
    ev.price = 100;
    ev.qty = 10;
    ev.recv_time = now_ns();

    batcher.submit(std::move(ev));

    std::this_thread::sleep_for(std::chrono::microseconds(30));
    }


    while (batcher.has_ready_batch()) {
        auto batch = batcher.pop_batch();
        engine.process_batch(batch);
        std::cout << "Processed batch of size " << batch.size() << "\n";
    }


*/


    std::cout << "=== MicroBatch Test ===\n";

MicroBatcher batcher(50'000); // 50 µs
MatchingEngine engine;

// Submit events
for (int i = 0; i < 6; i++) {
    OrderEvent ev;
    ev.type = EventType::NEW;
    ev.order_id = i + 1;
    ev.instrument = "ABC";
    ev.side = Side::BUY;
    ev.price = 100;
    ev.qty = 10;
    ev.recv_time = now_ns();

    batcher.submit(std::move(ev));
    std::this_thread::sleep_for(std::chrono::microseconds(20));
}

// 🔴 FORCE batch processing (this is the key)
auto batch = batcher.pop_batch();
engine.process_batch(batch);

std::cout << "Processed batch of size " << batch.size() << "\n";

}