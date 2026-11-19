# Project 47: Satellite Image Tile Processing Pipeline Scheduler

A C++20 geospatial batch-processing simulator. This starter contains the first vertical slice: validated job records, stable merge sort, deterministic scheduling order, a CLI demonstration, and tests.

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/project47_cli
```

## Current scheduling order

1. Higher priority first.
2. For equal priority, shorter estimated processing time first.
3. For ties, lower job ID first.

The merge sort is stable and runs in O(n log n) time with O(n) auxiliary memory. Job validation rejects a zero ID, an empty tile ID, and a negative estimated duration.

## Initial layout

```text
include/project47/models/job.hpp
include/project47/algorithms/merge_sort.hpp
include/project47/algorithms/job_order.hpp
src/main.cpp
tests/test_main.cpp
```

## Next implementation slice

Add `Tile`, `ComputeNode`, and dependency records, then quick sort and binary search behind the same interface. Keep algorithms independently testable before wiring them into the scheduler.
