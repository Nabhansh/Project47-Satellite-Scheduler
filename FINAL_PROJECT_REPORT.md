# Project 47 — Final Project Report

**Project:** Satellite Image Tile Processing Pipeline Scheduler  
**Implementation:** C++20  
**Submission:** Phase I and Phase II  
**Final verification environment:** Fedora Linux, GNU C++ 16.2.1, C++20 mode  
**Final verified test count:** 1,979 assertions

## 1. Executive summary

Project 47 is a C++20 geospatial workload scheduling **simulator**. It models satellite tile metadata, processing jobs, dependencies, compute nodes, storage nodes, and weighted storage/compute network links. It demonstrates how Data Structures and Algorithms (DAA) techniques can organize workloads, find dependency-safe execution order, assign eligible jobs to workers, estimate data-transfer routes, enforce modeled resource limits, and export execution reports.

The final Phase II implementation includes the required algorithm families, an integrated scheduler, normal/capacity-shock/stress scenarios, custom CSV workload input, CSV/JSON reporting, automated tests, and a benchmark runner. The final Fedora run reported 1,979 passing assertions and successful completion of the three bundled scenarios.

**Scope boundary:** The application simulates tile workloads. It does not decode GeoTIFF/COG raster pixels, execute raster transformations, transfer real files, or launch distributed worker processes.

## 2. Problem statement

A satellite imagery pipeline may receive many tile-processing jobs with different priorities, estimated processing durations, input sizes, prerequisites, and output requirements. Jobs compete for a limited number of compute workers and finite storage capacity. Data must be routed through a modeled network, while job dependencies must be respected.

The project models these decisions as algorithmic subproblems:

- workload organization and lookup;
- dependency validation and topological scheduling;
- interval/resource selection;
- shortest-path routing and network construction;
- compute-worker assignment by bipartite matching;
- capacity-constrained selection and exact-search comparisons;
- metadata pattern search;
- scenario reporting and empirical benchmarking.

## 3. Objectives

1. Implement the DAA algorithms specified by the Project 47 handbook using student-written C++20 code.
2. Represent tiles, jobs, compute resources, storage resources, dependencies, and network links with typed records and graph structures.
3. Integrate selected algorithms into one CSV-driven scheduling simulator.
4. Compare correctness and empirical behavior on small, medium, large, and adversarial inputs.
5. Demonstrate normal operation, a resource/capacity shock, and a large-data stress scenario.
6. Provide reproducible build, test, benchmark, documentation, and release instructions.

## 4. System design

### 4.1 Main records and structures

| Component | Responsibility |
|---|---|
| `Job` | Job ID, tile reference, priority, estimated duration, input size, status and prerequisites |
| `Tile` | XYZ identity, zoom, geographic metadata, estimated size and checksum metadata |
| `ComputeNode` | Worker identity, CPU/memory capacity, available resources and online status |
| `StorageNode` | Capacity, used/free bytes, bandwidth/region metadata and online state |
| Dependency graph | Directed prerequisite relationships and cycle detection |
| Weighted network graph | Modeled storage-to-compute and compute-to-storage transfer routes |
| Bipartite eligibility graph | Edges between ready jobs and workers that satisfy modeled constraints |

### 4.2 Scheduler execution flow

1. Load built-in or CSV workload records.
2. Validate IDs, tile references, sizes, resource values and dependency relationships.
3. Reject invalid records and cyclic dependency graphs.
4. Build a per-run route cache: one Dijkstra search from input storage and one from each online compute node to output storage.
5. Identify dependency-ready jobs and order them using the scheduling comparator.
6. Build job/worker eligibility edges based on worker availability, memory, and cached routes.
7. Run maximum bipartite matching so a worker receives at most one job in a scheduling wave.
8. Recheck changing storage availability and capacity before committing completions.
9. Mark feasible jobs complete and leave infeasible jobs deferred.
10. Update modeled storage usage, processing work, route cost and wave metrics.
11. Export per-job CSV and run-summary JSON reports.

The route cache assumes the network topology and edge weights stay unchanged during one scheduler invocation. Dynamic storage capacity and availability are checked when jobs are considered and committed; they are not cached.

For `W` online workers and a network with `V` vertices and `E` edges, route preparation performs at most `W + 1` heap-based Dijkstra searches, with time `O((W + 1)(V + E) log V)`. The scheduler's job/worker eligibility graph can contain up to `O(JW)` edges for `J` ready jobs. The implemented augmenting-path matching has a polynomial bound documented in `docs/COMPLEXITY_ANALYSIS.md`.

## 5. Algorithm coverage

The Phase II source includes the handbook algorithm set, grouped below. Some algorithms are reusable library demonstrations and test cases; not every algorithm is invoked in every scheduler run.

| Family | Implementations |
|---|---|
| Sorting/searching | Merge Sort, Quick Sort, iterative Binary Search, recursive Binary Search |
| Divide and conquer | Maximum Subarray, Fast Powering, Celebrity Discovery, classical matrix multiplication, Strassen matrix multiplication |
| Greedy | Activity Selection, Interval Partitioning, Huffman Coding |
| Graphs | Dijkstra, Prim, Kruskal, Bellman-Ford, Floyd-Warshall, Ford-Fulkerson/Edmonds-Karp, maximum bipartite matching, topological ordering and cycle detection |
| Dynamic programming | 0/1 Knapsack, Matrix Chain Multiplication, LCS, Optimal BST, bounded Held-Karp TSP |
| Backtracking/exact search | N-Queens, Subset Sum, FIFO Branch-and-Bound, LIFO Branch-and-Bound, least-cost Branch-and-Bound |
| Number/string algorithms | Chinese Remainder Theorem utility, Naive matching, Rabin-Karp, finite-automaton matching, KMP |

Detailed source mapping is in `Phase_II_20261119_1148/project47_starter/docs/ALGORITHM_COVERAGE.md`.

## 6. Demonstration scenarios and verified results

The final Phase II Fedora execution reported:

| Scenario | Jobs | Completed | Deferred | Waves | Processing work | Wave-barrier makespan estimate | Aggregate route cost |
|---|---:|---:|---:|---:|---:|---:|---:|
| Normal | 5 | 5 | 0 | 2 | 53.00 s | 27.00 s | 13.90 |
| Capacity shock | 5 | 2 | 3 | 2 | 17.00 s | 17.00 s | 5.00 |
| Stress | 128 | 128 | 0 | 16 | 1,013.00 s | 173.00 s | 345.60 |

A separate custom CSV run completed 3/3 jobs with no deferrals. The CLI writes scenario-specific `*_jobs.csv` and `*_report.json` files. These are deterministic simulator outputs based on the bundled sample workloads, not real satellite processing measurements.

### Makespan interpretation

`Estimated wave-barrier makespan` sums the longest estimated job duration in each wave. The current simulator finishes/commits a wave before constructing the next. This is a transparent estimate for the implemented barrier policy. It is not a measured wall-clock time, not a globally optimal schedule, and not an event-driven execution model. An event-driven scheduler could start newly unblocked jobs earlier and overlap work across dependency levels.

## 7. Testing and verification

The final Phase II source was built on the target Fedora machine using GNU C++ 16.2.1 in C++20 mode. The user-provided final logs show:

- Release CMake configuration and compilation succeeded.
- Strict-warning configuration/build succeeded.
- CTest passed: 1/1 test target.
- `project47_tests` reported **1,979 assertions passed**.
- Normal, capacity-shock, and stress CLI scenarios produced the expected completion/deferred counts.
- The custom CSV workload completed successfully.
- The benchmark executable completed and wrote `outputs/benchmark_results.csv`.
- The `RUN_STRICT=1 ./scripts/run_submission_checks.sh` path completed successfully.

The Phase II source package deliberately excludes compiled binaries and generated outputs. Rebuild it from source using the commands in the submission README.

## 8. Benchmark observations from the Fedora console capture

The benchmark executable uses repeated runs and records medians: three runs for sorting/string matching and five runs for scheduler scenarios. The following figures are copied from the user's final Fedora console output on 2026-11-19. They are local microsecond observations, not portable performance guarantees.

### Sorting — microseconds (Merge Sort / Quick Sort)

| Input pattern | n=1,000 | n=10,000 | n=50,000 |
|---|---:|---:|---:|
| Random | 29 / 22 | 442 / 397 | 2,734 / 2,219 |
| Sorted | 10 / 16 | 140 / 331 | 783 / 3,447 |
| Reverse-sorted | 10 / 11 | 128 / 236 | 914 / 2,268 |
| Duplicate-heavy | 15 / 4 | 260 / 67 | 1,760 / 398 |
| Organ-pipe | 10 / 14 | 131 / 237 | 888 / 1,550 |

### String matching — microseconds (Naive / Rabin-Karp / KMP / Finite Automaton)

| Text length | Observed timings |
|---:|---:|
| 1,000 | 18 / 7 / 3 / 11 |
| 10,000 | 116 / 49 / 14 / 21 |
| 50,000 | 586 / 254 / 62 / 83 |

### Exact search and scheduler — microseconds

| Workload | Time | Objective / completed | Search effort / deferred |
|---|---:|---:|---:|
| FIFO Branch-and-Bound | 37 | Value 545 | 303 nodes expanded |
| LIFO Branch-and-Bound | 153 | Value 545 | 2,447 nodes expanded |
| Least-cost Branch-and-Bound | 5 | Value 545 | 18 nodes expanded |
| Scheduler normal (5 jobs) | 15 | 5 completed | 0 deferred |
| Scheduler capacity shock (5 jobs) | 10 | 2 completed | 3 deferred |
| Scheduler stress (128 jobs) | 4,299 | 128 completed | 0 deferred |

The captured compiler string was GNU 16.2.1 (Red Hat 16.2.1-3), C++20 language mode (`202002`). The terminal transcript identifies the host as Fedora. CPU model, RAM capacity, kernel string and CMake version were not included in the captured output; collect them before claiming a fully reproducible hardware record. The original machine-generated `outputs/benchmark_results.csv` remains on the user's Fedora Phase II working copy and should be copied into the final evidence folder separately; this report does not pretend that the console table is the original CSV file.

## 9. Complexity and computational limits

The project documents algorithmic complexity, recurrence solutions and decision-problem classifications in:

- `docs/COMPLEXITY_ANALYSIS.md`
- `docs/REDUCTION_MAPPING.md`

Representative examples:

- Merge Sort: `O(n log n)` time, `O(n)` auxiliary memory.
- Quick Sort: average `O(n log n)`, worst `O(n^2)`.
- Dijkstra: `O((V+E) log V)` for the heap-based implementation.
- Bellman-Ford: `O(VE)`.
- Floyd-Warshall: `O(V^3)`.
- Ford-Fulkerson with BFS augmentations (Edmonds-Karp): `O(VE^2)` for integral capacities.
- 0/1 Knapsack: `O(nW)` pseudo-polynomial in capacity.
- Held-Karp TSP: `O(n^2 2^n)` time and `O(n 2^n)` space; the implementation is bounded to at most 18 cities.
- Branch-and-Bound Knapsack: exponential worst case, although pruning reduces the observed search on some instances.

The whole codebase should not be called NP-complete. Specific decision versions of Knapsack, Subset Sum and TSP are NP-complete; standard shortest path, MST, maximum flow and bipartite matching formulations are polynomial-time solvable. The reduction document states the exact decision forms and mappings.

## 10. Limitations and future work

The current scope is a simulator. A real geospatial pipeline would need:

- GeoTIFF/COG raster readers and writers;
- actual raster operations such as reprojection, resampling, mosaicking or statistics;
- physical worker processes or distributed execution;
- real data transfer and measured network latency;
- persistent queue/state, retry and idempotency handling;
- authentication, authorization, observability, deployment and operational recovery;
- event-driven scheduling for overlapping newly-ready jobs.

These are future production extensions and are not claimed as completed functionality.

## 11. Reproducible build and run commands

From `Phase_II_20261119_1148/project47_starter`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPROJECT47_ENABLE_STRICT_WARNINGS=ON
cmake --build build --clean-first --parallel "$(nproc)"
ctest --test-dir build --output-on-failure
./build/project47_tests
./build/project47_cli --scenario all --output-dir outputs
./build/project47_benchmarks outputs/benchmark_results.csv
RUN_STRICT=1 ./scripts/run_submission_checks.sh
```

## 12. Conclusion

Project 47 delivers a tested C++20 DAA implementation set and an integrated geospatial workload scheduling simulator. Its strongest contribution is the traceable progression from a small Phase I job-ordering starter to a Phase II simulator that combines dependency handling, worker matching, cached route planning, capacity checks, scenario evaluation, exportable reports, benchmarks and documented complexity analysis. The implementation and its boundaries are explicitly documented so the academic demonstration does not overstate production geospatial capabilities.
