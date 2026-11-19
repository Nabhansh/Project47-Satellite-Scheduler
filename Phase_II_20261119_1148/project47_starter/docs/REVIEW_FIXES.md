# Review Fixes Applied

## 1. Header portability

`include/project47/scheduler/scheduler.hpp` now includes `<cmath>` directly because it calls `std::isfinite`. It no longer relies on another header to include the declaration transitively.

## 2. Dijkstra route caching

The scheduler now builds a `RouteCache` once per workload execution:

- one Dijkstra search from the configured input storage node;
- one Dijkstra search from each online compute node to the configured output storage node.

The cached worker routes hold the path and distance for both transfer legs. Each job-worker eligibility check reuses these results. Dynamic checks, such as storage online status, source bytes, destination free capacity, and overflow protection, still run when the job is considered and committed.

For `W` online compute nodes, route preparation performs at most `W + 1` Dijkstra searches per scheduler invocation, instead of rerunning Dijkstra for each job-worker candidate and again after matching. For a graph with `V` vertices and `E` edges, the route-preparation work is `O((W + 1)(V + E) log V)` using the current binary-heap Dijkstra implementation, plus cached path copies/checks during matching.

The route cache assumes that the network topology and edge weights do not change during one scheduler invocation. If dynamic link failures or changing bandwidth are added later, cache invalidation or per-edge versioning will be required.

## 3. Submission packaging

The source-only packaging script is `scripts/package_submission.sh`. It excludes `.git`, `outputs/`, `build/`, and every `build-*` directory. The submission ZIP therefore contains source, tests, examples, documentation, CMake configuration, and scripts, but not compiled binaries or generated run output.

## 4. Phase I and Phase II snapshots

The combined hackathon package contains two separately timestamped folders:

- `Phase_I_20261118_1636/`: the original starter snapshot from the Phase 1 delivery. It contains the initial `Job` record, scheduling comparator, stable Merge Sort, basic CLI, and starter tests.
- `Phase_II_20261119_1148/`: the expanded final source with the full algorithm suite, integrated scheduler, reports, benchmarks, automated tests, and the review fixes in this document.

The Phase I folder is preserved from the original starter ZIP; it was not reverse-engineered from the final source.

## 5. Makespan interpretation

`estimated_makespan_seconds` is now described in the CLI and documentation as an **estimated wave-barrier makespan**. The scheduler commits one wave before it constructs the next. It therefore sums the longest estimated job duration in each wave. This is a conservative estimate for the implemented barrier-based policy, not an event-driven optimal schedule or a measured wall-clock makespan.

An event-driven scheduler could start a newly unblocked job as soon as its own prerequisites and a worker are available, overlapping it with unrelated work still running in an earlier wave. Such a scheduler could have a lower makespan. The current implementation intentionally keeps the simpler batch-wave policy for transparent DAA demonstrations.

## Verification performed for the reviewed Phase II source

- CMake Release build with strict warnings enabled (`-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`).
- CTest: passed.
- Project 47 assertions: 1,979 passed.
- Normal, capacity-shock, and stress CLI scenarios: passed.
- CLI now labels the makespan as `Estimated wave-barrier makespan`.
