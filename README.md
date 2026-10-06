# Particle Life

Particle Life is a deterministic C++17 simulation with five interacting species in a toroidal world.

The seed implementation intentionally checks every particle pair even though forces have a fixed interaction radius. This creates a clear optimization opportunity while the correctness test protects deterministic simulation behavior.

## Requirements

- CMake 3.16 or newer
- A C++17 compiler
- Python 3 for the benchmark wrapper
- Python Tkinter and a graphical display for the (default-on) visualization pass; see below for headless behavior

## Artemis commands

Compile:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel
```

Test:

```bash
ctest --test-dir build --output-on-failure
```

Benchmark:

```bash
python3 tools/benchmark.py
```

The benchmark measures 30 simulation frames with 2,200 particles after five warmup frames and writes `simulation_fps` to `artemis_results.json`.

## Visualization

The measured benchmark is always followed by a visual pass — 3,500 particles for 10 seconds by default — showing the resulting simulation:

```bash
python3 tools/benchmark.py
```

Change the duration and visual workload without changing the measured workload:

```bash
python3 tools/benchmark.py --duration 20 --visual-particles 4000
```

Pass `--no-visualize` to skip it entirely, e.g. for a faster local loop:

```bash
python3 tools/benchmark.py --no-visualize
```

The visualization is never part of the timed benchmark section, so it cannot affect `simulation_fps`. On a runner with no graphical display, the visualization pass fails and prints a warning to stderr instead of failing the whole command — the measured `artemis_results.json` from the timed section is written first and is unaffected either way.
