# Project 47 Hackathon Submission Package

This archive preserves the two requested timestamped development snapshots and includes final hand-in documentation.

## Submission contents

- `Phase_I_20261118_1636/`: genuine early Project 47 starter snapshot.
- `Phase_II_20261119_1148/`: final reviewed C++20 implementation.
- `PHASE_COMPARISON.md`: documented Phase I-to-Phase II improvement.
- `FINAL_PROJECT_REPORT.md`: consolidated academic report.
- `VIVA_QA.md`: viva questions and speaking notes.
- `BENCHMARK_EVIDENCE.md`: benchmark observations captured from the Fedora console.
- `GIT_RELEASE.md`: instructions to commit both phases and create annotated tag `v1.0.0`.
- `evidence/`: destination for the original Fedora benchmark CSV and machine metadata.

## Build and verify Phase II on Fedora

```bash
cd Phase_II_20261119_1148/project47_starter
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPROJECT47_ENABLE_STRICT_WARNINGS=ON
cmake --build build --clean-first --parallel "$(nproc)"
ctest --test-dir build --output-on-failure
./build/project47_tests
./build/project47_cli --scenario all --output-dir outputs
./build/project47_benchmarks outputs/benchmark_results.csv
RUN_STRICT=1 ./scripts/run_submission_checks.sh
```

The final Fedora verification reported 1,979 passing assertions. The normal, capacity-shock and 128-job stress scenarios completed with the results documented in `FINAL_PROJECT_REPORT.md`.

## Important scope boundary

Project 47 is a geospatial tile workload **scheduling simulator**. It models tile metadata, jobs, compute/storage resources, dependencies, route selection, capacity checks and reports. It does not decode GeoTIFF/COG raster pixels or execute raster transformations.

## Git release and benchmark evidence

Follow `GIT_RELEASE.md` from the submission root. Copy the original local `outputs/benchmark_results.csv` and machine metadata into `evidence/` before committing. The console-derived timing table in `BENCHMARK_EVIDENCE.md` is a summary, not a replacement for the generated CSV.
