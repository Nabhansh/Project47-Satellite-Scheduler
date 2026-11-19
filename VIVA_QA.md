# Project 47 — Viva Questions and Model Answers

Use these as speaking notes. Explain the idea first, then point to the relevant source module and live CLI demonstration. Do not claim the simulator processes actual raster pixels.

## Project and architecture

### 1. What problem does Project 47 solve?
It models scheduling decisions for satellite tile-processing workloads. Each tile is represented by metadata and each processing task by a Job. The simulator validates dependencies, assigns ready jobs to available compute nodes, models storage routes and capacity, and exports completion/deferred reports.

### 2. Is this a real satellite image-processing system?
No. It is a geospatial workload scheduling simulator. It models tile identifiers, sizes, jobs, resources and network links. It does not decode GeoTIFF/COG pixels or execute raster transformations.

### 3. What is the main execution pipeline?
CSV or demo workload loading → validation → dependency graph checks → route cache → ready-job selection → job/worker eligibility graph → maximum matching → live storage-capacity checks → complete/defer jobs → CSV/JSON reports.

### 4. Why did you use C++20?
The project is an algorithm-focused C++ implementation. C++20 provides standard containers, priority queues, filesystem support and a modern build target while allowing the algorithms and data structures to be implemented directly.

### 5. What are the main domain records?
Job, Tile, ComputeNode and StorageNode. Graph structures represent job dependencies, weighted transfer links, flow capacity and bipartite job/worker eligibility.

## Algorithms and choices

### 6. Why implement both Merge Sort and Quick Sort?
They illustrate different time/space trade-offs. Merge Sort is stable with predictable `O(n log n)` time and `O(n)` auxiliary space. Quick Sort is in-place apart from recursion stack and is average `O(n log n)`, but worst-case `O(n^2)`. The benchmark compares behavior on random, sorted, reverse, duplicate-heavy and organ-pipe inputs.

### 7. Why is Binary Search only valid on sorted data?
Its correctness depends on the ordering relation. Each comparison discards half the remaining interval. If the input is not sorted under the same comparator, the discarded half may contain the target.

### 8. Why use a dependency graph and topological order?
A directed edge represents a prerequisite. Topological ordering gives an order where every prerequisite appears before its dependent job. A cycle means no valid execution order exists, so the workload is rejected.

### 9. Why use Dijkstra for transfer routes?
The modeled network uses non-negative edge costs. Dijkstra efficiently finds shortest paths in that setting. If negative edge weights were permitted, Bellman-Ford would be the suitable single-source alternative, subject to negative-cycle handling.

### 10. What is the difference between Prim and Kruskal?
Both find a minimum spanning tree for a connected weighted undirected graph. Prim grows one tree by choosing the cheapest edge leaving the current tree. Kruskal sorts edges and adds an edge when it joins two different components, using a disjoint-set structure.

### 11. Why use maximum bipartite matching?
One side contains ready jobs and the other contains online compute workers. An edge exists when a worker is eligible for a job. Maximum matching assigns as many jobs as possible in one wave while ensuring a worker is not assigned twice in that wave.

### 12. Why are Bellman-Ford and Floyd-Warshall also included?
They satisfy the syllabus and demonstrate alternative shortest-path problem settings. Bellman-Ford supports negative edge weights and detects reachable negative cycles. Floyd-Warshall computes all-pairs shortest paths. The scheduler's current routing graph uses Dijkstra because its link costs are non-negative.

### 13. What does Ford-Fulkerson demonstrate?
It models maximum transferable flow through a capacitated network. The implementation uses BFS augmenting paths (Edmonds-Karp), so the standard integral-capacity bound is `O(VE^2)`.

### 14. Why compare FIFO, LIFO and least-cost Branch-and-Bound?
They explore the same exact search space using different frontiers: queue, stack and priority queue. On the benchmark instance they find the same optimal value but expand different node counts. This exposes how search order and pruning affect practical effort without changing the exponential worst-case bound.

### 15. What is the role of KMP?
KMP searches for a metadata pattern without restarting comparisons from the beginning after a mismatch. Its time is `O(n+m)` for text length `n` and pattern length `m`, using a prefix/failure table.

## Scheduler and review improvements

### 16. What did you change after code review?
Added a direct `<cmath>` include for `std::isfinite`; cached Dijkstra route results once per scheduler invocation; clarified that makespan is a wave-barrier estimate; excluded build binaries and generated outputs from source packages; and prepared separate timestamped Phase I and Phase II snapshots.

### 17. How does route caching work?
The topology and edge weights are assumed fixed during a single scheduler run. The scheduler computes one Dijkstra result from input storage and one from each online compute node to output storage. Candidate job/worker pairs reuse these path and cost results. Storage online state and free capacity are still checked dynamically before a job is committed. With `W` online workers, there are at most `W+1` Dijkstra searches per run.

### 18. Is the makespan a real parallel makespan?
No. It is explicitly called an estimated wave-barrier makespan. It sums the longest estimated job duration in each scheduling wave because the simulator commits one wave before creating the next. An event-driven scheduler could overlap jobs from different dependency levels and may produce a lower makespan. The current value is not wall-clock execution time or a global optimum.

### 19. What happens during the capacity-shock scenario?
The scenario reduces available compute/storage resources. The final demo completes 2 of 5 jobs and reports 3 as deferred. Deferred jobs are not falsely reported as completed.

### 20. What did the stress test demonstrate?
The bundled stress workload contains 128 jobs. The final Fedora run completed all 128 jobs across 16 scheduling waves, with no deferred jobs.

## Complexity and evaluation

### 21. Is the whole project NP-complete?
No. Complexity classes apply to precisely defined decision problems, not a whole code repository. Knapsack, Subset Sum and TSP decision variants are NP-complete. Shortest path, MST, maximum flow and bipartite matching have polynomial-time standard formulations.

### 22. What is the complexity of Held-Karp TSP?
`O(n^2 2^n)` time and `O(n 2^n)` space. The implementation is deliberately bounded to at most 18 cities because its state space grows exponentially.

### 23. What does the benchmark memory column represent?
It is an analytical estimate of selected auxiliary structures, not measured resident-set size or peak process RAM. It excludes allocator overhead and other process memory.

### 24. What are the final test results?
The final Fedora Phase II run passed CTest and reported 1,979 assertions. The normal, capacity-shock and stress scenarios ran successfully. The strict validation script also completed successfully.

### 25. What would be required to make this production-grade?
Real GeoTIFF/COG I/O and raster operators, actual worker processes, real transfer, persistence, retries/idempotency, cancellation, security, monitoring and deployment infrastructure. An event-driven scheduler would also improve concurrency beyond barrier-based waves.

## Live demonstration order

1. Show `README.md` and the architecture in `docs/SYSTEM_DESIGN.md`.
2. Run `ctest --test-dir build --output-on-failure` and `./build/project47_tests`.
3. Run `./build/project47_cli --scenario all --output-dir outputs/demo`.
4. Open one scenario JSON and one per-job CSV report.
5. Run `./build/project47_benchmarks outputs/benchmark_results.csv`.
6. Show `docs/ALGORITHM_COVERAGE.md`, `docs/COMPLEXITY_ANALYSIS.md`, and `docs/REDUCTION_MAPPING.md`.
7. End by stating the simulator boundary accurately.
