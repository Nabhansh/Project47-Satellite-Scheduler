# Fedora Evidence Folder

Copy these files from the verified Phase II working directory before the final Git commit:

- `fedora_benchmark_results_2026-11-19.csv` — the original `outputs/benchmark_results.csv` created by the benchmark executable.
- `fedora_machine_metadata_2026-11-19.txt` — CPU, RAM, OS/kernel, GNU C++ and CMake versions.

The console-captured timings are summarized in `../BENCHMARK_EVIDENCE.md`. That Markdown file is not a replacement for the original CSV.

From `Phase_II_20261119_1148/project47_starter`, run:

```bash
mkdir -p ../../evidence
cp outputs/benchmark_results.csv ../../evidence/fedora_benchmark_results_2026-11-19.csv
{
  echo "Capture date: 2026-11-19"
  echo "--- OS/kernel ---"; uname -a
  echo "--- CPU ---"; lscpu
  echo "--- RAM ---"; free -h
  echo "--- C++ compiler ---"; c++ --version
  echo "--- CMake ---"; cmake --version
} > ../../evidence/fedora_machine_metadata_2026-11-19.txt
```
