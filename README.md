# FairOrder: A Latency-Fair Stock Order Matching Engine

## The Motivation
Modern electronic markets often reward participants with microsecond-level advantages, encouraging an expensive and socially inefficient latency arms race. This project explores a latency-equalized exchange matching engine that intentionally reduces unfair timing advantages while preserving price discovery.

The core idea is to introduce a fairness layer between order ingress and matching, ensuring that orders arriving within a small, controlled time window are treated equivalently rather than strictly by raw arrival time.

## Key Ideas
- Micro-batching for ingress fairness
- Deterministic batch-level tie-breaking
- Replayable and auditable execution
- Thread-ready C++ core (CMake, zero runtime deps)

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
