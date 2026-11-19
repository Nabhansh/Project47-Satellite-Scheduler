# Project 47: Complexity-Class Analysis and Reduction Mapping

## Decision versions and complexity classes

Complexity classes apply to precisely stated decision problems. The project contains several different optimization and decision subproblems; it is inaccurate to label the entire codebase simply “NP-complete.”

| Problem represented in Project 47 | Decision form | Classification (standard integer-encoded formulation) |
|---|---|---|
| 0/1 tile selection / Knapsack | Is there a subset with total size at most `B` and total value at least `Q`? | NP-complete; optimization version NP-hard. |
| Subset Sum | Does a subset of tile sizes sum exactly to target `T`? | NP-complete. |
| Travelling Salesperson | Is there a tour visiting every city once with total cost at most `B`? | NP-complete; optimization version NP-hard. |
| General resource-constrained scheduling | Can jobs be assigned and ordered within capacities and a makespan bound? | NP-hard in general; restricted cases may be polynomial. |
| Shortest path, MST, max flow, bipartite matching | Is there a path/tree/flow/matching satisfying a threshold or cardinality? | Polynomial-time solvable for the implemented standard formulations. |

An NP certificate is a proposed subset, tour, or schedule. Under finite integer/rational encodings, the certificate can be checked in polynomial time by summing sizes/costs, checking uniqueness, verifying precedence, and validating resource limits. This establishes membership in NP for the stated decision variants. NP-hardness follows from the reductions below.

## Reduction 1: 0/1 Knapsack to capacity-constrained tile selection

Given a Knapsack instance with items `(weight_i, value_i)`, capacity `B`, and target value `Q`:

1. Create one tile candidate `tile_i` for each item.
2. Set the tile's storage/memory requirement to `weight_i`.
3. Set its scheduling benefit or priority score to `value_i`.
4. Set the available resource capacity to `B`.
5. Ask whether the scheduler can select a set with total requirement at most `B` and total benefit at least `Q`.

The mapping takes polynomial time and preserves yes/no answers. Therefore the tile-selection decision problem is at least as hard as 0/1 Knapsack and is NP-hard (and in NP under the stated finite encoding, hence NP-complete).

## Reduction 2: Subset Sum to exact tile-bundle selection

Given positive integers `a_1, ..., a_n` and target `T`, create one tile with byte/MB requirement `a_i` for each integer. Ask whether a selected tile bundle has total size exactly `T`. A subset sums to `T` if and only if the corresponding tile bundle has exact size `T`. This is a polynomial many-one reduction, so exact bundle selection is NP-hard.

## Reduction 3: PARTITION to two-worker makespan scheduling

Given positive processing times `p_1, ..., p_n` with total `2B`, create `n` independent jobs with processing times `p_i` and two identical workers. Ask whether all jobs can be assigned so that neither worker's total load exceeds `B` (equivalently, the makespan is at most `B`). Such an assignment exists exactly when the numbers can be partitioned into two subsets each summing to `B`. This maps PARTITION to a restricted scheduler decision problem in polynomial time, proving NP-hardness of that scheduling variant.

## What the project demonstrates

- Polynomial-time methods (Dijkstra, MST, matching, max flow, dynamic-programming shortest paths) solve the tractable graph subproblems.
- Exact dynamic programming is pseudo-polynomial for Knapsack and exponential for TSP; practical input bounds matter.
- Backtracking and branch-and-bound are exact search techniques, not polynomial-time guarantees. Node counts and pruning effectiveness must be measured.
- The integrated scheduler uses polynomial-time dependency ordering, matching, and shortest paths to build feasible waves. It does not solve a globally optimal generalized scheduling objective.

## Recommended production strategy

Use a hybrid design:

1. Validate inputs and reject cyclic dependency graphs before scheduling.
2. Build dependency-safe ready waves with topological reasoning.
3. Use matching and shortest-path routing for feasible assignments and data movement.
4. Use greedy methods for fast interval/resource heuristics where exactness is not required.
5. Invoke exact Knapsack, branch-and-bound, or Held-Karp only for bounded subproblems (the supplied TSP implementation caps at 18 cities).
6. For larger optimization instances, apply a documented heuristic or approximation and report its bound/quality instead of pretending an exponential exact method will scale.
7. Retain feasibility checks as the final authority: resource limits, dependencies, storage availability, and route existence must be verified before marking a job complete.

The current program is a simulator. A production geospatial deployment would also require raster I/O, worker execution, persistent state, fault recovery, authentication, and observability.
