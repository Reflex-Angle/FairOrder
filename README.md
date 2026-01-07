# FairOrder: A Latency-Fair Stock Order Matching Engine

## The Motivation
Modern electronic markets often reward participants with microsecond-level advantages, encouraging an expensive and socially inefficient latency arms race. This project explores a latency-equalized exchange matching engine that intentionally reduces unfair timing advantages while preserving price discovery.

The core idea is to introduce a fairness layer between order ingress and matching, ensuring that orders arriving within a small, controlled time window are treated equivalently rather than strictly by raw arrival time.

## Key Features

- **Dual Matching Modes**: Compare naive price-time priority vs. latency-fair batching
- **Simulated Traders**: Multiple traders with configurable network latencies (5µs to 100µs)
- **Fairness Metrics**: Real-time computation of fairness index and latency advantage reduction
- **Interactive CLI**: Run simulations, experiments, and view results in real-time
- **Comparative Experiments**: Side-by-side comparison of matching modes
- **High Performance**: Zero-dependency C++17 implementation with CMake

## Quick Start

### Build

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Run

```bash
./engine
```

Build instructions are provided so that readers can compile and run the prototype locally (no precompiled binaries are distributed with this repository) to explore its behavior and evaluate the fairness mechanisms implemented.

The interactive CLI will start. Type `help` for available commands or `menu` for the main menu.


## Architecture

```
┌──────────────┐
│  CLI Layer   │  ← Interactive interface
└──────┬───────┘
       │
┌──────▼────────┐
│ Order Ingress │  ← Simulated traders with latencies
└──────┬────────┘
       │
┌──────▼────────┐
│ MicroBatcher  │  ← Groups orders by time window
└──────┬────────┘
       │
┌──────▼────────┐
│ MatchingEngine│  ← Processes batches/orders
└──────┬────────┘
       │
┌──────▼────────┐
│  OrderBook    │  ← Maintains order book state
└──────┬────────┘
       │
┌──────▼────────┐
│   Metrics     │  ← Tracks fairness statistics
└───────────────┘
```

## How It Works

### Naive Mode (Price-Time Priority)

1. Orders are processed immediately upon arrival
2. Priority: Best price first, then earliest `recv_time`
3. **Result**: Fast traders (low latency) always win ties

### Fair Mode (Latency-Fair Batched)

1. Orders arriving within a time window (e.g., 100µs) are grouped into batches
2. Within each batch, orders are sorted by price, then by `order_id` (ignoring `recv_time`)
3. Batches are processed deterministically
4. **Result**: Traders with similar prices have equal chances regardless of latency

### Example Scenario

**Setup**: Two traders submit identical buy orders at price $100
- **FastTrader**: 5µs latency
- **SlowTrader**: 50µs latency

**Naive Mode**:
- FastTrader's order arrives first → executes first
- Win rate: FastTrader 95%, SlowTrader 5%

**Fair Mode** (100µs batch window):
- Both orders arrive within the same batch
- Priority determined by `order_id` (fair)
- Win rate: FastTrader 52%, SlowTrader 48%


## Current Status

- [x] Micro-batching layer
- [x] Deterministic batching
- [x] Order book and matching logic
- [x] Naive vs. fair mode comparison
- [x] Trader simulation with configurable latencies
- [x] Fairness metrics and statistics
- [x] Interactive CLI interface
- [x] Comparative experiments
- [ ] Replay / audit logging
- [ ] Web UI

## Techn Stack

- **Language**: C++
- **Build System**: CMake 3.16+
- **Dependencies**: None (standard library only)
- **Platform**: Cross-platform (Windows, Linux, macOS)


> **Disclaimer**  
> This project is a research and educational prototype.  
> It is not intended for production trading, live markets, or financial use.