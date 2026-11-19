# Phase I to Phase II Improvement Record

## Snapshot provenance

| Snapshot | Folder timestamp | Basis |
|---|---|---|
| Phase I | `Phase_I_20261118_1636` | Original Phase 1 starter ZIP and its original file timestamps |
| Phase II | `Phase_II_20261119_1148` | Completed source after the final review fixes |

The Phase I files are preserved from the earlier starter archive. They are not a reconstructed or backdated copy of Phase II.

## What Phase I contained

- A validated `Job` record with ID, tile ID, priority, estimated duration, input bytes, and status.
- A deterministic scheduling comparator: priority descending, duration ascending, then job ID ascending.
- Stable Merge Sort for scheduling order.
- A small CLI that displayed the sorted sample jobs.
- A basic test executable for job validation and sorting.
- Initial build instructions and a suggested next implementation slice.

## Improvements delivered in Phase II

| Area | Phase I baseline | Phase II improvement |
|---|---|---|
| Domain model | Job record | Job, Tile, ComputeNode, StorageNode, dependencies, and network records |
| Sorting/searching | Stable Merge Sort | Merge Sort, Quick Sort, iterative and recursive Binary Search |
| Data structures | Basic records and arrays | Directed dependency graph, weighted network graph, flow network, and bipartite graph |
| Greedy algorithms | Not present | Activity Selection, Interval Partitioning, Huffman Coding, Dijkstra, Prim, and Kruskal |
| Dynamic programming | Not present | Knapsack, Matrix Chain, LCS, Optimal BST, Bellman-Ford, Floyd-Warshall, and Held-Karp TSP |
| Divide and conquer | Not present | Maximum Subarray, Fast Powering, Celebrity Discovery, and classical/Strassen multiplication |
| Backtracking / exact search | Not present | N-Queens, Subset Sum, FIFO/LIFO/least-cost Branch-and-Bound |
| Network and text algorithms | Not present | Ford-Fulkerson, maximum bipartite matching, CRT, Naive, Rabin-Karp, finite automaton, and KMP |
| Integrated behavior | Sorted job display | Dependency-aware, capacity-constrained tile workload scheduling with worker assignment and route planning |
| Workload input/output | Hard-coded CLI example | CSV workload input and per-job CSV / run-summary JSON exports |
| Scenarios | No integrated scenarios | Normal, capacity-shock, and 128-job stress scenarios |
| Evaluation | Starter tests | 1,978 assertions, benchmark executable, strict-warning build, complexity analysis, and design documents |
| Portability / scalability fixes | No route cache or direct `<cmath>` include | Explicit `<cmath>` include and per-run Dijkstra route caching |

## Observable Phase I to Phase II change

Phase I could sort a small set of jobs and print the order. Phase II accepts a workload, validates tile and infrastructure records, respects dependencies, matches eligible jobs to distinct workers, checks storage capacity, calculates transfer routes, simulates scenarios, and exports machine-readable reports. It also provides independent implementations and benchmarks for the required DAA algorithm families.

## Honest scope boundary

Phase II is a geospatial **workload scheduling simulator**. It models tile identifiers and byte sizes, jobs, compute/storage nodes, links, and scheduling decisions. It does not decode GeoTIFF/COG pixels or execute raster transformation kernels. Those would be separate production extensions, not features claimed by this academic deliverable.
