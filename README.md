# FairOrder: A Latency-Fair Stock Order Matching Engine

## The Motivation
Modern electronic markets often reward participants with microsecond-level advantages, encouraging an expensive and socially inefficient latency arms race. This project explores a latency-equalized exchange matching engine that intentionally reduces unfair timing advantages while preserving price discovery.

The core idea is to introduce a fairness layer between order ingress and matching, ensuring that orders arriving within a small, controlled time window are treated equivalently rather than strictly by raw arrival time.

## Key Ideas
- Micro-batching for ingress fairness
- Deterministic batch-level tie-breaking
- Replayable and auditable execution
- Thread-ready C++ core (CMake, zero runtime dependencies)

These techniques are inspired by real-world mechanisms such as exchange “speed bumps” and frequent batch auctions, but the focus here is on adaptive schemes that minimize latency arbitrage without fully abandoning continuous price discovery.

## Evaluation Goals
The engine is designed to support empirical evaluation, including:
- Latency advantage reduction: Measuring how much microsecond-level advantages are neutralized.
- Market quality impact: Effects on spread, execution price, and VWAP.
- Throughput and determinism: Ensuring high event rates with replayable, auditable outcomes.

## Current Status
- [x] Micro-batching layer
- [x] Deterministic batching
- [ ] Tie-breaking logic
- [ ] Order book and matching
- [ ] Replay / audit logging

## Build Instructions
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
Build instructions are provided so that readers can compile and run the prototype locally (no precompiled binaries are distributed with this repository) to explore its behavior and evaluate the fairness mechanisms implemented.

> **Disclaimer**  
> This project is a research and educational prototype.  
> It is not intended for production trading, live markets, or financial use.
