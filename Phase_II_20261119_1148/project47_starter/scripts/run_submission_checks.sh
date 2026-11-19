#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

JOBS="${BUILD_JOBS:-2}"

cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel "$JOBS"
ctest --test-dir build-release --output-on-failure
./build-release/project47_tests
./build-release/project47_cli --scenario all --output-dir outputs/final-demo
./build-release/project47_cli --scenario normal \
  --jobs examples/sample_jobs.csv --output-dir outputs/final-custom
./build-release/project47_benchmarks outputs/benchmark_results.csv

if [[ "${RUN_STRICT:-0}" == "1" ]]; then
  cmake -S . -B build-strict -DCMAKE_BUILD_TYPE=Release \
    -DPROJECT47_ENABLE_STRICT_WARNINGS=ON
  cmake --build build-strict --parallel "$JOBS"
  ctest --test-dir build-strict --output-on-failure
fi

printf '\nSubmission checks complete. Inspect outputs/ and archive benchmark_results.csv with machine details.\n'
