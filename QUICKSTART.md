# Quick Start Guide

## Building the Project

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running the Demo

```bash
./engine
```

## Example Session

```
> help                    # See all commands
> menu                    # Show interactive menu
> mode fair               # Set to latency-fair mode
> window 100us            # Set batch window to 100 microseconds
> simulate 1000           # Run simulation with 1000 orders
> metrics                 # View fairness metrics
> experiment              # Run comparative experiment (naive vs fair)
> quit                    # Exit
```

## Understanding the Output

### Fairness Index
- **0.0**: Completely unfair (one trader dominates)
- **1.0**: Perfectly fair (equal win rates)

### Latency Advantage Reduction
- Shows percentage reduction in latency advantage
- Higher is better (more fair)

### Trader Statistics
Shows for each trader:
- Orders submitted
- Orders executed
- Win rate
- Number of trades won

## Key Commands

| Command | Description |
|---------|-------------|
| `simulate N` | Run simulation with N orders |
| `experiment` | Compare naive vs fair modes |
| `mode <naive\|fair>` | Set matching mode |
| `window <time>` | Set batch window (e.g., 50us, 1ms) |
| `metrics` | Show fairness metrics |
| `book` | Show order book state |
| `reset` | Reset engine and metrics |

## Tips

1. **Start with an experiment**: Run `experiment` to see the difference between modes
2. **Try different batch windows**: Smaller windows (10-50µs) are more responsive but less fair
3. **Compare trader latencies**: The system includes traders with 5µs, 25µs, 50µs, and 100µs latencies
4. **Run multiple simulations**: Results vary, so run several to see consistent patterns

## Expected Results

In **naive mode**, fast traders (5µs) typically win 90%+ of collisions.

In **fair mode**, win rates should be much more balanced (45-55% for each trader).

