# Fedora Benchmark Evidence — Project 47 Phase II

**Capture date:** 2026-11-19  
**Host identification:** Fedora Linux (terminal prompt `charlie@fedora`)  
**Compiler:** GNU C++ 16.2.1, Red Hat build `16.2.1-3`  
**Language mode:** C++20 (`__cplusplus = 202002`)  
**Benchmark build:** Release workflow executed by `scripts/run_submission_checks.sh`  
**Source of figures:** console output supplied from the final Fedora `RUN_STRICT=1 ./scripts/run_submission_checks.sh` run.

These are microsecond observations printed by the benchmark program. The benchmark methodology uses medians over three runs for sorting/string searching and five runs for scheduler scenarios. Results depend on hardware, compiler, system load and build flags; they are not portable performance guarantees.

## Sorting timings (µs)

Each cell is `Merge Sort / Quick Sort`.

| Input | n=1,000 | n=10,000 | n=50,000 |
|---|---:|---:|---:|
| Random | 29 / 22 | 442 / 397 | 2,734 / 2,219 |
| Sorted | 10 / 16 | 140 / 331 | 783 / 3,447 |
| Reverse | 10 / 11 | 128 / 236 | 914 / 2,268 |
| Duplicates | 15 / 4 | 260 / 67 | 1,760 / 398 |
| Organ-pipe | 10 / 14 | 131 / 237 | 888 / 1,550 |

## String matching timings (µs)

Algorithm order: Naive / Rabin-Karp / KMP / Finite Automaton.

| Text length | Timings |
|---:|---:|
| 1,000 | 18 / 7 / 3 / 11 |
| 10,000 | 116 / 49 / 14 / 21 |
| 50,000 | 586 / 254 / 62 / 83 |

## Branch-and-Bound and scheduler timings

| Workload | Time (µs) | Result | Search effort |
|---|---:|---|---:|
| FIFO Branch-and-Bound | 37 | Value 545 | 303 nodes expanded |
| LIFO Branch-and-Bound | 153 | Value 545 | 2,447 nodes expanded |
| Least-cost Branch-and-Bound | 5 | Value 545 | 18 nodes expanded |
| Normal scheduler (5 jobs) | 15 | 5 completed, 0 deferred | — |
| Capacity-shock scheduler (5 jobs) | 10 | 2 completed, 3 deferred | — |
| Stress scheduler (128 jobs) | 4,299 | 128 completed, 0 deferred | — |

## Interpretation

- Quick Sort is faster on the captured random and duplicate-heavy cases, while Merge Sort is much faster on sorted, reverse and organ-pipe cases at larger sizes. This is consistent with Quick Sort's sensitivity to input order and pivot behavior; its documented theoretical worst case remains `O(n^2)`.
- KMP is fastest in this captured repetitive near-match text workload. The relative timings are specific to this text, compiler and machine.
- The three exact-search strategies return the same value but explore different numbers of nodes. Least-cost expands the fewest nodes in this instance; this is not a guarantee for all instances.
- The 128-job stress scheduler completed all jobs in the captured run.

## Machine information still to append

The console transcript did not include CPU model, RAM capacity, kernel string or CMake version. Collect them on the Fedora machine before archiving the raw CSV as a fully reproducible benchmark record:

```bash
lscpu
free -h
uname -a
c++ --version
cmake --version
```

The actual benchmark CSV is generated on the Fedora machine at `Phase_II_20261119_1148/project47_starter/outputs/benchmark_results.csv`. It was not attached to this chat, so this document preserves the console-captured figures without claiming to be a copy of that original CSV. Copy the original CSV and machine metadata into the top-level `evidence/` folder before the final Git commit/release.
