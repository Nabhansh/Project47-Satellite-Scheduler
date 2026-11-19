# Project 47: Complexity and Recurrence Analysis

This document records the asymptotic costs of the implemented algorithms. Let:

- `n` be the number of jobs, tiles, values, or text characters as appropriate;
- `V` and `E` be graph vertex and edge counts;
- `m` be a pattern length;
- `W` be the integer knapsack capacity;
- `k` be the number of symbols in a Huffman alphabet.

The bounds describe the algorithms, not the end-to-end scheduler's measured wall time. Data structures, output size, and implementation details can add costs.

## Sorting, searching, and divide-and-conquer

| Algorithm | Time | Auxiliary space | Notes |
|---|---:|---:|---|
| Merge Sort | `O(n log n)` | `O(n)` | Stable; allocates a merge buffer. |
| Quick Sort | Average `O(n log n)`, worst `O(n^2)` | `O(log n)` stack in this implementation | Median-of-three, three-way partition; recurses on the smaller partition to bound stack depth. |
| Iterative Binary Search | `O(log n)` | `O(1)` | Input must be sorted under the comparator. |
| Recursive Binary Search | `O(log n)` | `O(log n)` | Returns the first matching index. |
| Maximum Subarray (divide-and-conquer) | `O(n log n)` | `O(log n)` | The crossing scan is linear at each recursion level. |
| Celebrity Discovery (divide-and-conquer) | `O(n)` relation checks | `O(log n)` recursion | Candidate reduction is `2T(n/2)+O(1)` followed by linear verification. |
| Exponentiation by Squaring | `O(log |p|)` | `O(1)` | `p` is the exponent. |
| Classical Matrix Multiplication | `O(n^3)` | `O(n^2)` output | Square matrices in the supplied implementation. |
| Strassen Matrix Multiplication | `O(n^(log2 7))` asymptotically | `O(n^2)` live matrix storage by implementation | Uses a classical cutoff and pads to a power of two; padding and constants can make it slower for small matrices. |

## Greedy, graph, and string algorithms

| Algorithm | Time | Auxiliary space | Notes |
|---|---:|---:|---|
| Activity Selection | `O(n log n)` | `O(n)` | Sorting dominates. |
| Interval Partitioning | `O(n log n)` | `O(n)` | Heap-based resource assignment. |
| Huffman Coding | `O(k log k)` | `O(k)` | For `k` distinct symbols. |
| Dijkstra | `O((V+E) log V)` | `O(V+E)` | Non-negative edge weights; priority-queue implementation. |
| Prim | `O(E log V)` | `O(V+E)` | Priority-queue implementation. |
| Kruskal | `O(E log E)` | `O(V)` | Edge sort plus disjoint-set structure. |
| Bellman-Ford | `O(VE)` | `O(V)` | Supports negative edges and detects reachable negative cycles. |
| Floyd-Warshall | `O(V^3)` | `O(V^2)` | All-pairs shortest paths. |
| Ford-Fulkerson with BFS (Edmonds-Karp) | `O(VE^2)` | `O(V+E)` | Integral capacities assumed for the standard bound. |
| Maximum Bipartite Matching (augmenting paths) | `O(VE)` | `O(V+E)` | Bound for the DFS augmenting-path implementation. |
| Naive String Search | `O(nm)` | `O(occ)` output | `occ` is the number of matches returned. |
| Rabin-Karp | Expected `O(n+m)`, worst `O(nm)` | `O(occ)` output plus `O(1)` rolling state | Hash collisions are verified by character comparison. |
| KMP | `O(n+m)` | `O(m+occ)` | Prefix table plus output positions. |
| Finite-Automaton Search | `O(256m+n)` | `O(256m+occ)` | Full byte alphabet; transition table is explicit. |
| Chinese Remainder Theorem utility | `O(k log M)` arithmetic operations, approximately | `O(k)` | `k` congruences; arithmetic bit complexity depends on integer width. |

## Dynamic programming, backtracking, and exact search

| Algorithm | Time | Auxiliary space | Notes |
|---|---:|---:|---|
| 0/1 Knapsack | `O(nW)` | `O(nW)` in the stored reconstruction table | Pseudo-polynomial in numeric capacity `W`. |
| Matrix Chain Multiplication | `O(n^3)` | `O(n^2)` | `n` matrices. |
| Longest Common Subsequence | `O(ab)` | `O(ab)` | Input sequence lengths `a` and `b`; reconstruction table retained. |
| Optimal BST | `O(n^3)` | `O(n^2)` | Standard interval DP. |
| N-Queens backtracking | `O(n!)` upper bound | `O(n)` search state, excluding stored solutions | Search tree prunes conflicting placements; outputting all solutions can dominate. |
| Subset Sum backtracking | `O(2^n)` | `O(n)` search depth, excluding result storage | Worst-case exhaustive search. |
| Branch-and-Bound Knapsack | `O(2^n)` worst case | `O(2^n)` worst-case frontier | Bounds can greatly reduce explored nodes but do not improve the worst-case class. |
| Held-Karp TSP | `O(n^2 2^n)` | `O(n 2^n)` | Bounded to 18 cities in this project to limit memory. |

## Recurrence relations and solutions

### Merge Sort

`T(n) = 2T(n/2) + cn`, with `T(1) = Θ(1)`.

At each recursion level the total merge work is `cn`. There are `log2(n)` levels, so `T(n) = Θ(n log n)`.

### Quick Sort

Balanced/average case: `T(n) = 2T(n/2) + cn = Θ(n log n)`.

Worst case: `T(n) = T(n-1) + cn = Θ(n^2)`. Median-of-three and three-way partitioning help on common sorted and duplicate-heavy inputs but do not eliminate the theoretical worst case.

### Binary Search and Fast Powering

`T(n) = T(n/2) + c = Θ(log n)`. Each step halves the remaining interval or exponent.

### Maximum Subarray (Divide-and-Conquer)

`T(n) = 2T(n/2) + cn = Θ(n log n)`. Two half-problems are solved and the crossing solution is scanned in linear time.

### Celebrity Discovery (Divide-and-Conquer)

Candidate reduction follows `T(n) = 2T(n/2) + O(1) = O(n)`. A final linear verification ensures the surviving candidate knows nobody and is known by everyone else.

### Strassen Matrix Multiplication

`T(n) = 7T(n/2) + cn^2`. By the Master Theorem, `T(n) = Θ(n^(log2 7))`, approximately `Θ(n^2.8074)`.

### Matrix Chain, LCS, and Optimal BST

Matrix Chain and Optimal BST evaluate `O(n^2)` intervals with `O(n)` split choices: `Θ(n^3)`. LCS evaluates one cell per pair of input positions: `Θ(ab)`.

### Knapsack and Held-Karp TSP

0/1 Knapsack's table has `nW` states/transitions, hence `Θ(nW)`. Held-Karp has `2^n` subsets and up to `n` endpoints and transitions per state, hence `Θ(n^2 2^n)`.

### Graph Algorithms

Bellman-Ford performs up to `V-1` edge-relaxation passes: `Θ(VE)`. Floyd-Warshall evaluates all intermediate/source/destination triples: `Θ(V^3)`. Edmonds-Karp's BFS augmentation bound is `Θ(VE^2)`.

## Empirical interpretation

The benchmark executable reports medians over repeated local runs and checks outputs for equivalence. Timings are machine-, compiler-, build-, and load-dependent. Estimated auxiliary-memory values in the CSV are algorithmic estimates, not process RSS measurements. Do not use a single run to claim universal superiority of one algorithm.

## Integrated scheduler route planning

Let `W` be the number of online compute nodes. The scheduler builds the route cache once per workload: one Dijkstra run from input storage and one Dijkstra run from each online compute node. With the current heap-based implementation, route preparation is `O((W+1)(V+E) log V)` time. Cached path storage is `O(WV)` in the worst case because each worker retains an input and output path of at most `V` vertices. The job/worker eligibility graph can contain up to `O(JW)` edges for `J` ready jobs; maximum matching is then run on that graph. Storage capacity and online state are checked dynamically and are not part of the route cache.
