# Latency-Fair Stock Order Matching Engine

## Overview
This project implements a latency-equalized stock order matching engine
using micro-batching and deterministic tie-breaking to combat latency arbitrage.

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
