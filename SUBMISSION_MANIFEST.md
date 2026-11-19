# Submission Manifest

This folder is the academic hand-in root for Project 47.

| Item | Purpose |
|---|---|
| `Phase_I_20261118_1636/` | Preserved genuine early starter snapshot |
| `Phase_II_20261119_1148/` | Reviewed final C++20 implementation |
| `PHASE_COMPARISON.md` | Phase I to Phase II improvement record |
| `FINAL_PROJECT_REPORT.md` | Consolidated project report |
| `VIVA_QA.md` | Viva preparation questions and speaking notes |
| `BENCHMARK_EVIDENCE.md` | Console-captured Fedora performance results and caveats |
| `GIT_RELEASE.md` | Instructions to create the repository commit and `v1.0.0` tag on Fedora |
| `evidence/` | Place to preserve the original generated benchmark CSV and machine metadata |

## Verified final Phase II baseline

- GNU C++ 16.2.1, C++20 mode.
- Release and strict-warning builds completed on Fedora.
- CTest passed.
- `project47_tests` reported 1,979 assertions passed.
- Normal: 5/5 completed.
- Capacity shock: 2/5 completed, 3 deferred.
- Stress: 128/128 completed.
- Custom CSV workload: 3/3 completed.
- `RUN_STRICT=1 ./scripts/run_submission_checks.sh` completed successfully.

The raw `outputs/benchmark_results.csv` was generated on the user's Fedora machine but was not attached to this chat. Copy it into `evidence/` using the commands in `GIT_RELEASE.md` before committing the final repository.
