# Project 47: Benchmark Methodology

## Running the benchmark suite

```bash
./build/project47_benchmarks outputs/benchmark_results.csv
```

The executable writes a machine-readable CSV and prints the same measurements to the terminal. It benchmarks:

- Merge Sort and Quick Sort for 1,000, 10,000, and 50,000 integer keys;
- random, sorted, reverse-sorted, duplicate-heavy, and organ-pipe input patterns;
- Naive, Rabin-Karp, KMP, and finite-automaton search on repetitive near-match text;
- FIFO, LIFO, and least-cost branch-and-bound on the same 22-item knapsack instance, checked against the dynamic-programming optimum;
- the integrated normal, capacity-shock, and 128-job stress scheduler scenarios.

Sorting and string-search routines are run three times and the median is recorded. Scheduler scenarios are run five times and the median is recorded. Output equivalence, sortedness, and exact-search objective values are checked during every run; benchmark execution fails when an algorithm's output is inconsistent.

## Reproducibility record

When preparing the final submission, record the following alongside the CSV:

- CPU model and physical/logical core count;
- RAM capacity;
- operating system and kernel;
- compiler name and version;
- CMake version;
- build type and compiler flags;
- date/time and whether the machine was under other load.

A convenient way to collect part of the record on Fedora is:

```bash
lscpu
free -h
uname -a
c++ --version
cmake --version
```

Use a Release build for timing. Do not compare Debug and Release timing results. Run multiple complete benchmark sessions if reporting a trend.

## Interpreting memory columns

The CSV's `estimated_auxiliary_bytes` column contains analytical estimates for selected algorithmic structures (for example Merge Sort's buffer or the finite automaton's transition table). It is not measured resident-set size and does not include allocator overhead, code, input, output vectors, or the operating system. Do not present it as measured peak RAM.

## Expected qualitative observations

- Merge Sort should show predictable `n log n` scaling but allocates a linear buffer.
- Quick Sort often benefits from in-place partitioning and three-way handling of duplicates; its median-of-three pivot does not remove its `O(n^2)` theoretical worst case.
- KMP avoids repeated character comparisons on repetitive near-match text and remains linear in text plus pattern length.
- The finite automaton has linear scan time after preprocessing but spends `O(256m)` memory on a byte-alphabet transition table.
- FIFO, LIFO, and least-cost Branch-and-Bound are run against the same knapsack instance; their objective value must equal the DP oracle, while expanded/generated node counts expose search effort.
- Exact scheduler simulation time depends on graph density, worker count, job count, dependency structure, and the matching implementation.

No performance conclusion should be based on one machine's microsecond figures alone. Include algorithmic complexity, correctness checks, workload shape, and resource usage in the written comparison.
