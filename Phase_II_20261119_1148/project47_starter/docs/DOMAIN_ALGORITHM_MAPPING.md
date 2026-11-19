# Project 47: Domain-to-Algorithm Mapping

| Satellite pipeline concern | Model/data | Algorithm or method | Expected decision/output |
|---|---|---|---|
| Organize incoming tile jobs | Job records and tile IDs | Merge Sort / Quick Sort | Priority- and duration-based workload ordering |
| Find a tile or job quickly | Sorted tile metadata keys | Iterative / recursive Binary Search | First matching tile index |
| Validate processing order | Job prerequisites | DAG cycle detection and topological ordering | Reject cycles and construct safe dependency order |
| Select non-overlapping processing windows | Job intervals | Activity Selection | Maximum compatible window subset under the standard interval model |
| Assign simultaneous jobs to worker slots | Job start/end intervals | Interval Partitioning | Minimum number of interchangeable slots |
| Compress repeated metadata symbols | Tile/event metadata frequencies | Huffman Coding | Prefix-free codebook and round-trip decoding |
| Move tiles across storage/compute links | Weighted network | Dijkstra | Lowest modeled transfer-cost route for non-negative links |
| Plan a low-cost infrastructure backbone | Compute/storage network | Prim / Kruskal | Minimum spanning tree cost and selected links |
| Allocate limited memory/storage budget | Tile weight and value estimates | 0/1 Knapsack; FIFO/LIFO/least-cost B&B | Exact bounded selection and search-strategy comparison |
| Optimize operation order | Matrix dimensions for image operators | Matrix Chain DP | Minimum scalar multiplication count and parenthesization |
| Compare metadata strings or event signatures | Metadata sequences | LCS | Longest shared subsequence |
| Optimize a search index over metadata keys | Key and gap frequencies | Optimal BST | Minimum expected search cost under the supplied probabilities |
| Find paths with signed modeled costs | Directed weighted graph | Bellman-Ford / Floyd-Warshall | Single-source and all-pairs shortest paths; negative-cycle detection |
| Visit a bounded set of infrastructure endpoints | Cost matrix | Held-Karp TSP | Minimum visit-all tour for up to 18 cities |
| Validate constraint-search implementations | Small exact instances | N-Queens / Subset Sum | Complete solution sets or exact subset witnesses |
| Estimate tile-transfer throughput | Capacity network | Ford-Fulkerson (Edmonds-Karp BFS) | Maximum flow through storage/compute links |
| Assign tile jobs to distinct workers | Bipartite eligibility graph | Maximum Bipartite Matching | Maximum number of simultaneous feasible assignments |
| Combine modular schedules | Congruence constraints | Chinese Remainder Theorem | Combined residue and modulus when compatible |
| Find repeated metadata signatures | Event text and pattern | Naive / Rabin-Karp / Finite Automaton / KMP | Match positions with comparable correctness results |
| Demonstrate capacity and scale behavior | Bundled/custom CSV workloads | Integrated scheduler scenarios and benchmarks | Completion/deferred counts, waves, estimated work/makespan, route cost and timings |

**Interpretation:** algorithm examples are algorithmic models of satellite tile processing decisions. They are not claims that the simulator decodes imagery or executes real raster transformations. The route weights, processing durations, tile sizes, and resource capacities are supplied workload data or deterministic demo values.
