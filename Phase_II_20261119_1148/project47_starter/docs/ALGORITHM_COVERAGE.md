# Project 47: Handbook Algorithm Coverage

This table maps the handbook's ten-line specification to the supplied source modules and demonstration evidence.

| Handbook requirement | Implemented modules | Main source / evidence |
|---|---|---|
| Typed tile, job, compute, storage, dependency and network models | Job, Tile, ComputeNode, StorageNode, directed/undirected weighted graphs, dependency DAG | `include/project47/models/`, `include/project47/graphs/`, `tests/test_main.cpp` |
| Workload scheduling, repeated metadata-pattern detection, constraints and service indicators | CSV workload loader, dependency-aware scheduler, resource checks, route costs, makespan estimates, four text matchers | `include/project47/scheduler/`, CLI and scheduler tests |
| Merge Sort, Quick Sort, iterative/recursive Binary Search, tile metadata lookup | Stable Merge Sort, three-way Quick Sort, lower-bound-style iterative/recursive binary searches | `include/project47/algorithms/`, sorting/search tests |
| Maximum Subarray, Fast Powering, Celebrity Discovery, classical and Strassen matrix multiplication | Divide-and-conquer maximum subarray, exponentiation by squaring, divide-and-conquer celebrity reduction and a linear elimination alternative, classical and Strassen products | `divide_and_conquer.hpp`, `celebrity.hpp`, `matrix_multiplication.hpp` |
| Activity Selection, Interval Partitioning, Huffman, Dijkstra, Prim, Kruskal | Greedy interval algorithms, Huffman codebook, shortest paths, two MST implementations | `activity_selection.hpp`, `interval_partitioning.hpp`, `huffman_coding.hpp`, graph headers |
| Knapsack, Matrix Chain, LCS, Optimal BST, Bellman-Ford, Floyd-Warshall, bounded TSP | Dynamic-programming modules including Held-Karp TSP (up to 18 cities) | `knapsack.hpp`, `matrix_chain.hpp`, `lcs.hpp`, `optimal_bst.hpp`, graph headers, `travelling_salesperson.hpp` |
| N-Queens, subset/backtracking, FIFO/LIFO/least-cost branch-and-bound | N-Queens, subset sum, three branch-and-bound frontiers with common objective and bound | `backtracking.hpp`, `branch_and_bound.hpp`, tests and CLI |
| Ford-Fulkerson and maximum bipartite matching | Edmonds-Karp BFS augmentations and augmenting-path matching | `flow_network.hpp`, `bipartite_matching.hpp` |
| Chinese Remainder Theorem; Naive, Rabin-Karp, finite automaton, KMP | Modular congruence solver and four equivalent pattern matchers | `chinese_remainder.hpp`, `string_matching.hpp` |
| Recurrences, asymptotics, empirical comparisons, correctness, complexity classes, reductions and production strategy | Complexity analysis, benchmark runner, reductions, tests, system-design and recommendation documentation | `docs/COMPLEXITY_ANALYSIS.md`, `docs/REDUCTION_MAPPING.md`, `benchmarks/`, `tests/` |

## Important interpretation notes

- The bounded Held-Karp implementation intentionally caps input at 18 cities because its `O(n 2^n)` state storage grows exponentially.
- Strassen and classical matrix multiplication use signed 64-bit integer elements. Inputs must be chosen so intermediate arithmetic stays within the `int64_t` range.
- The scheduler's estimated processing/makespan metrics are modeled values, not execution timings measured on real compute nodes.
- “Detect repeated metadata patterns” is demonstrated by the four string-search implementations over the simulated metadata/event text.
- This program models tile identifiers, metadata, sizes, and routes. It is not a raster pixel-processing library.


## Scheduler implementation notes

- Dijkstra routes are cached per scheduler invocation: one search from input storage plus one search per online compute node. Candidate job-worker pairs reuse these results rather than rerunning shortest paths. Dynamic storage capacity checks are not cached.
- `estimated_makespan_seconds` is a wave-barrier estimate (sum of per-wave maximum job durations). It is conservative relative to an event-driven schedule that overlaps independent work across waves; it is not an optimal or measured makespan.
