# Project 47: System Design

## Purpose and boundary

Project 47 is a portable C++20 simulator for a satellite-image tile batch-processing pipeline. It models jobs, tile metadata, dependencies, compute workers, storage nodes, network links, resource limits, and algorithmic scheduling decisions. It does not read or transform raster pixels.

## Main entities

- **Tile:** XYZ tile identity, zoom, geographic bounds, estimated resolution, byte size, and checksum metadata.
- **Job:** unique job ID, tile reference, priority, estimated processing duration, input bytes, status, and prerequisite relationships.
- **ComputeNode:** node identity, CPU and memory capacity, free memory, processing estimate, and online/offline status.
- **StorageNode:** capacity, used bytes, bandwidth metadata, region, and online state.
- **Network graph:** weighted undirected links used by Dijkstra for modeled input and output routes.
- **Dependency graph:** directed acyclic graph used to validate prerequisites and derive a safe execution order.

## Execution flow

1. Parse command-line options and (when supplied) load a CSV workload.
2. Construct or load tile and infrastructure records.
3. Validate unique IDs, tile references, byte-size agreement, infrastructure records, route vertices, storage roles, and dependency acyclicity.
4. Build a per-run route cache: one Dijkstra search from input storage and one from each online compute node. Reuse those path/cost results for all job-worker eligibility checks and assignments.
5. Find jobs whose prerequisites have completed.
6. Sort ready jobs by the project priority comparator.
7. Build a bipartite eligibility graph between ready jobs and online workers with sufficient memory and valid cached transfer routes.
8. Run maximum bipartite matching so one worker is assigned to at most one job in the current wave.
9. Recheck dynamic source/destination availability and destination capacity before committing each completion.
10. Update storage accounting, completion state, route costs, wave count, processing estimate, and wave-barrier makespan estimate.
11. Leave infeasible jobs deferred; export per-job CSV and run-summary JSON.

## Key invariants

- A job is completed only when prerequisites are complete, a worker is assigned, both routes exist, and output storage has sufficient capacity.
- A worker receives at most one job per wave.
- A dependency cycle is rejected before execution.
- Every job references a known tile whose byte size agrees with the job input.
- Deferred jobs are reported as deferred, not silently marked successful.

## Reports and metrics

`*_jobs.csv` contains per-job outcome, worker, route endpoints, wave, and route cost. `*_report.json` contains scenario totals, per-job route details, deferred job identifiers, and storage usage. `estimated_processing_seconds` sums job duration estimates. `estimated_makespan_seconds` sums the maximum job estimate in each scheduling wave. This is a conservative wave-barrier estimate for the implemented batch policy, not an event-driven optimal makespan or wall-clock measurement. An asynchronous scheduler could overlap jobs from different dependency levels where resources permit. Transfer latency is represented only by graph-edge cost.

The route cache is built once per run. It performs one Dijkstra search from the input storage and one from each online compute node, yielding at most `1 + W` searches for `W` online workers. Job-worker eligibility checks reuse cached path and cost data, while storage capacity and availability are rechecked dynamically because destination usage changes as jobs complete.

## Failure handling

Invalid CSV, invalid records, dependency cycles, unknown nodes, invalid capacities, and impossible resource accounting raise errors and cause a non-zero CLI result. Resource shocks or unavailable routes leave affected jobs deferred. Output exporters create the destination directory and report I/O failures.

## Production gaps

A production geospatial product would need a raster reader/writer (for example GeoTIFF/COG support), real tile transformation operators, actual worker processes, network transfer, persistent queue/state, retry/idempotency controls, cancellation, security, monitoring, and deployment infrastructure. Those are outside this PBL algorithm simulator's current scope.
