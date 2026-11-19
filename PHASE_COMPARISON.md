# Project 47: Phase I to Phase II Improvement Record

## Timestamped snapshots

| Folder | Timestamp | Purpose |
|---|---|---|
| `Phase_I_20261118_1636/` | 2026-11-18 16:36 | Genuine early starter snapshot |
| `Phase_II_20261119_1148/` | 2026-11-19 11:48 Asia/Kolkata | Completed, reviewed implementation |

The Phase I source was preserved from the original first starter ZIP. It has not been recreated from Phase II or backdated.

## Comparison

| Area | Phase I | Phase II improvement |
|---|---|---|
| Domain model | Job record | Job, Tile, ComputeNode, StorageNode, dependencies, and transfer network |
| Core algorithms | Stable Merge Sort and scheduling comparator | Merge Sort, Quick Sort, iterative/recursive Binary Search, and the full handbook algorithm set |
| Graphs | None | Dependency DAG, weighted graphs, shortest paths, MST, flow, and matching |
| Optimization | None | Greedy, dynamic programming, backtracking, and three Branch-and-Bound strategies |
| Text/number algorithms | None | Naive, Rabin-Karp, finite automaton, KMP, and Chinese Remainder utility |
| Product behavior | Sort and print four sample jobs | CSV-driven geospatial workload simulator with dependency ordering, matching-based compute assignment, routing, capacity checks, and reports |
| Scenarios | None | Normal, capacity-shock, and 128-job stress scenarios |
| Evaluation | Starter tests | Benchmark suite, complexity/reduction documentation, and 1,979 assertions in the reviewed build |
| Engineering | Basic standalone headers | Explicit standard-library includes, per-run Dijkstra route caching, source-only packaging script, and clarified metrics |

## Concrete improvement

Phase I could validate jobs, sort them and display the order. Phase II validates a complete modeled workload, determines dependency-ready jobs, builds eligible job/worker edges, finds a matching, reuses cached shortest paths, reserves storage capacity, marks jobs completed or deferred, and exports execution reports. It also provides independent implementations and test evidence for the DAA algorithm families.

## Scope boundary

Phase II is a geospatial workload **simulator**. It models tile identifiers and byte sizes, jobs, compute/storage nodes, links, and scheduling decisions. It does not decode GeoTIFF/COG pixels or execute raster transformation kernels.
