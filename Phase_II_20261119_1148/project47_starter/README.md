# Project 47: Satellite Image Tile Processing Pipeline Scheduler

A portable C++20 geospatial batch-processing **simulator** for scheduling satellite-image tile workloads across compute and storage nodes. It includes the algorithm families listed in the Project 47 brief and integrates them into one dependency-aware scheduling run.

## Current status

Phases 1–9 are implemented in this package. Phase 9 closes the remaining divide-and-conquer and bounded-TSP coverage gaps, adds a reproducible benchmark executable, and includes complexity, recurrence, reduction, system-design, demonstration, and submission documents.

## Features by phase

1. **Core foundation:** typed `Job` records, validation, CMake, and stable merge sort.
2. **Sorting and searching:** student implementations of Quick Sort and iterative/recursive binary search.
3. **Geospatial data structures:** `Tile`, `ComputeNode`, `StorageNode`, dependency DAG, cycle checks, and topological sorting.
4. **Greedy and graph algorithms:** activity selection, interval partitioning, Huffman coding, Dijkstra, Prim, and Kruskal.
5. **Dynamic programming:** 0/1 knapsack, matrix-chain multiplication, LCS, optimal BST, Bellman-Ford, and Floyd-Warshall.
6. **Backtracking and branch-and-bound:** N-Queens, subset sum, FIFO/LIFO/least-cost branch-and-bound with a fractional-knapsack upper bound.
7. **Remaining algorithms:** Ford-Fulkerson (Edmonds-Karp BFS), maximum bipartite matching, Chinese Remainder Theorem, naive matching, Rabin-Karp, finite automaton, and KMP.
8. **Integrated scheduler:** dependency-aware waves, matching-based job-to-worker assignment, Dijkstra input/output transfer routes, storage capacity checks, workload CSV loading, and CSV/JSON report export.

## Build and run on Fedora/Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --clean-first -j"$(nproc)"
ctest --test-dir build --output-on-failure
./build/project47_tests
```

Build and run the benchmark suite (writes machine-specific measurements):

```bash
./build/project47_benchmarks outputs/benchmark_results.csv
```

Run every integrated demonstration scenario and write reports to `outputs/`:

```bash
./build/project47_cli --scenario all --output-dir outputs
```

Run one scenario:

```bash
./build/project47_cli --scenario normal
./build/project47_cli --scenario capacity-shock
./build/project47_cli --scenario stress
```

The CLI defaults to `--scenario all`. `normal` and `capacity-shock` use the same five jobs and dependency graph; the shock scenario takes two workers offline and limits remaining output storage. `stress` uses 128 independent jobs and eight workers.

### Load a job workload from CSV

```bash
./build/project47_cli --scenario normal \
  --jobs examples/sample_jobs.csv \
  --output-dir outputs/custom
```

CSV columns:

```csv
id,tile_id,priority,estimated_seconds,input_bytes,prerequisites
201,tile_demo_01,5,8.5,4000000,
202,tile_demo_02,4,11.0,6000000,
203,tile_demo_03,3,6.0,3000000,201;202
```

The `prerequisites` field is optional; when present, multiple prerequisite IDs are separated by semicolons. CSV quoting and escaped quotes are supported. Each job's `input_bytes` must be a positive byte count matching the generated tile record. The scheduler validates job IDs, tile references, infrastructure, storage roles, dependencies, and dependency cycles before scheduling.

### Reports

Each run writes two files named for its scenario:

- `<scenario>_jobs.csv`: one row per completed or deferred job, including worker, storage nodes, wave, routes, and costs.
- `<scenario>_report.json`: summary metrics, per-job routes, deferred job IDs, and final storage usage.

The scheduler uses maximum bipartite matching to avoid assigning two jobs to the same worker within a wave. A job becomes eligible only after all prerequisites complete. Dijkstra is used to route data from the configured input storage to a worker and from that worker to the configured output storage. Since the network topology is immutable during one run, route planning caches one Dijkstra result from the input storage and one from each online compute node. Candidate job-worker pairs then reuse the cached paths and costs; changing output-storage capacity is still checked dynamically before a job is committed. This reduces shortest-path computations from repeated work per job-worker pair to at most `1 + W` Dijkstra runs per scheduler invocation, where `W` is the number of online workers. Output storage capacity is reserved as jobs complete. Jobs that cannot run because of worker, route, or storage constraints remain deferred rather than being marked complete.

`estimated_processing_seconds` is the sum of job estimates. `estimated_makespan_seconds` is a **wave-barrier estimate**: it sums the longest estimated job duration in each scheduling wave. This matches the simulator's current barrier-based policy, where the next wave is considered only after the previous wave is committed. It is conservative relative to an event-driven schedule that could start an independent or newly-unblocked job as soon as its own prerequisites and worker are available, even while unrelated jobs from an earlier wave are still running. It is not a measured or optimal parallel makespan, and it excludes transfer latency and real machine execution. `route_cost` is the sum of configured graph-edge weights along the input and output paths.

## Phase 8 demonstration baseline

The package's tested sample scenarios produce:

| Scenario | Jobs | Completed | Deferred | Waves | Estimated processing work | Wave-barrier makespan estimate |
|---|---:|---:|---:|---:|---:|---:|
| Normal | 5 | 5 | 0 | 2 | 53 s | 27 s |
| Capacity shock | 5 | 2 | 3 | 2 | 17 s | 17 s |
| Stress | 128 | 128 | 0 | 16 | 1,013 s | 173 s |

These are deterministic simulator results from the bundled sample data, not production performance claims.

## Strict warnings and sanitizers

```bash
cmake -S . -B build-strict -DCMAKE_BUILD_TYPE=Release \
  -DPROJECT47_ENABLE_STRICT_WARNINGS=ON
cmake --build build-strict -j"$(nproc)"
ctest --test-dir build-strict --output-on-failure

cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g0" \
  -DPROJECT47_ENABLE_SANITIZERS=ON
cmake --build build-sanitize -j"$(nproc)"
ctest --test-dir build-sanitize --output-on-failure
```

## Source layout

```text
include/project47/
  algorithms/   # sorting, searching, divide-and-conquer, greedy, DP, BnB, string/number theory
  graphs/       # dependency, weighted/directed graphs, flow, matching, shortest paths, MST
  models/       # Job, Tile, ComputeNode, StorageNode
  scheduler/    # integrated scheduler, CSV workload reader, CSV/JSON report exporters
src/            # algorithm showcase and scenario CLI
tests/          # assertion-based unit, integration, and regression tests
benchmarks/     # reproducible sorting, string-search and scheduler benchmark runner
docs/           # design, complexity, reductions, benchmark method, demo and checklist
scripts/        # end-to-end submission validation script
examples/       # sample workload CSV
```

## Phase 9: algorithm closure and submission analysis

The final package also implements the divide-and-conquer modules and bounded TSP entry required by the ten-line project brief:

- Maximum Subarray (divide-and-conquer)
- Fast Powering (exponentiation by squaring)
- Celebrity Discovery (divide-and-conquer reduction and final verification)
- Classical and Strassen matrix multiplication
- Held-Karp dynamic programming for TSP instances up to 18 cities

The benchmark executable compares Merge Sort and Quick Sort on random, sorted, reverse, duplicate-heavy, and organ-pipe datasets; compares the four string matchers on repetitive near-match text; compares FIFO/LIFO/least-cost Branch-and-Bound against a DP optimum; and times the integrated scenarios. It records medians in CSV and checks correctness during measurement. See `docs/BENCHMARK_METHODOLOGY.md`.

Additional submission documents are in `docs/`: `SYSTEM_DESIGN.md`, `DOMAIN_ALGORITHM_MAPPING.md`, `ALGORITHM_COVERAGE.md`, `COMPLEXITY_ANALYSIS.md`, `REDUCTION_MAPPING.md`, `BENCHMARK_METHODOLOGY.md`, `DEMONSTRATION_SCRIPT.md`, and `FINAL_SUBMISSION_CHECKLIST.md`. Run `scripts/run_submission_checks.sh` for release, scenario, custom-CSV, and benchmark checks. Add `RUN_STRICT=1` to include the strict-warning build.

## Scope and limitations

This deliverable is a batch-processing **simulator**, not a production raster-processing engine. It models tile metadata, compute/storage resources, job dependencies, assignment, and route selection. It does not decode GeoTIFF/COG imagery, execute image transformations, communicate with real worker machines, or physically transfer tile files. The scheduler's time and route metrics are modeled estimates based on the supplied workload and graph weights.


## Create a clean source-only ZIP

After verification, create a submission archive without compiled binaries, local build directories, `.git`, or generated `outputs/` files:

```bash
./scripts/package_submission.sh
```

The script writes `../project47_starter_source_submission.zip` by default and checks that build/output artifacts were excluded. Keep the benchmark CSV generated on the demonstration machine separately with the CPU, RAM, OS, and compiler details.
