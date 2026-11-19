# Project 47: Final Demonstration Script (5–7 minutes)

## 1. Problem and scope (30–45 seconds)

“Project 47 models a satellite-image tile processing pipeline. Tile workloads arrive as jobs. Each job has priority, estimated duration, input size, dependencies, and a tile metadata record. Jobs must be assigned to constrained compute nodes, moved through a storage network, and reported with evidence of correctness and resource use.”

Clarify that this deliverable is a C++20 **scheduler simulator**. It models geospatial tile metadata and workload movement; it does not decode or transform real GeoTIFF raster pixels.

## 2. Architecture (45–60 seconds)

Show the source layout:

- `models/`: Job, Tile, ComputeNode, StorageNode;
- `graphs/`: dependency, weighted, directed, flow, and bipartite graphs;
- `algorithms/`: sorting/searching, greedy, divide-and-conquer, DP, backtracking, branch-and-bound, string and number-theory algorithms;
- `scheduler/`: workload validation, dependency-aware scheduling, and report export.

Explain the core flow: validate → build the per-run Dijkstra route cache → identify ready jobs → construct job/worker eligibility → maximum matching → recheck live storage capacity → complete/defer → export. The cache runs Dijkstra once from input storage and once per online compute node, rather than once per job-worker pair.

## 3. Run correctness tests (30 seconds)

```bash
ctest --test-dir build --output-on-failure
./build/project47_tests
```

State the assertion count printed by the current checked-out build. Do not quote a previous count if tests have changed.

## 4. Demonstrate all scenarios (90 seconds)

```bash
./build/project47_cli --scenario all --output-dir outputs/demo
```

Point out the expected bundled baseline. Call the time field the **wave-barrier makespan estimate**: the simulator adds the longest job duration in each wave because it does not overlap different waves. An event-driven scheduler could do better by starting newly-unblocked work while unrelated earlier work is still running. This is not an optimal or measured wall-clock makespan:

- Normal: 5/5 jobs complete, 0 deferred, 2 waves.
- Capacity shock: 2/5 complete, 3 deferred, 2 waves.
- Stress: 128/128 complete, 0 deferred, 16 waves.

Explain that the capacity-shock scenario takes compute workers offline and restricts output storage. Deferred jobs are reported honestly rather than marked successful.

## 5. Load an external workload and inspect reports (45–60 seconds)

```bash
./build/project47_cli --scenario normal \
  --jobs examples/sample_jobs.csv \
  --output-dir outputs/custom
```

Show `outputs/custom/normal_jobs.csv` and `outputs/custom/normal_report.json`. Point out the job outcome, worker assignment, routes, scheduling wave, route cost, and storage accounting.

## 6. Benchmark and complexity evidence (60 seconds)

```bash
./build/project47_benchmarks outputs/benchmark_results.csv
```

Show the generated CSV. Explain the median-of-runs method and the test data shapes. Distinguish analytical memory estimates from actual process memory. Connect the observed trends to the recurrence and complexity document.

## 7. Close with algorithmic decisions (30–45 seconds)

Explain the hybrid production recommendation: use polynomial-time graph algorithms and dependency waves for routine execution; use exact DP/branch-and-bound only on small bounded allocation decisions; use heuristics or approximation for larger NP-hard instances. End by stating the production boundary: real raster I/O, workers, transfer, persistence, retries, and observability are outside this simulator.
