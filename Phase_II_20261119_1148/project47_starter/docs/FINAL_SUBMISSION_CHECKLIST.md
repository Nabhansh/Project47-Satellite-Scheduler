# Project 47: Final Submission Checklist

## Source and build

- [ ] C++20 source is in the release repository.
- [ ] CMake configures and builds from a clean directory.
- [ ] CTest passes and the test executable prints its assertion count.
- [ ] Strict warnings build passes (`-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`).
- [ ] Sanitizer build passes where the host toolchain supports AddressSanitizer and UndefinedBehaviorSanitizer.
- [ ] No build artifacts, personal paths, API keys, or local machine files are committed.
- [ ] The hand-in contains timestamped `Phase_I_...` and `Phase_II_...` folders.
- [ ] Phase I is the preserved early starter snapshot, not a backdated copy of Phase II.
- [ ] `scripts/package_submission.sh` creates a source-only ZIP with build and output folders excluded.

## Functional demonstration

- [ ] Normal scenario completes 5/5 jobs.
- [ ] Capacity-shock scenario leaves infeasible jobs deferred rather than marking them complete.
- [ ] Stress scenario completes the bundled 128-job workload.
- [ ] Custom CSV workload loads and produces CSV and JSON reports.
- [ ] Report files are valid and correspond to the demonstrated run.
- [ ] All algorithm modules have a test or CLI demonstration.

## Algorithm coverage

- [ ] Sorting/searching: Merge Sort, Quick Sort, iterative and recursive Binary Search.
- [ ] Divide-and-conquer: Maximum Subarray, Fast Powering, Celebrity Discovery, Classical and Strassen Matrix Multiplication.
- [ ] Greedy: Activity Selection, Interval Partitioning, Huffman Coding.
- [ ] Graphs: Dijkstra, Prim, Kruskal, Bellman-Ford, Floyd-Warshall, Ford-Fulkerson, Bipartite Matching.
- [ ] Dynamic programming: 0/1 Knapsack, Matrix Chain, LCS, Optimal BST, bounded Held-Karp TSP.
- [ ] Backtracking / exact search: N-Queens, Subset Sum, FIFO/LIFO/least-cost Branch-and-Bound.
- [ ] Number and string algorithms: Chinese Remainder Theorem, Naive, Rabin-Karp, finite automaton, KMP.

## Written evidence

- [ ] System design and limitations are documented.
- [ ] Complexity table and recurrence solutions are included.
- [ ] Complexity-class analysis and reduction mappings are included.
- [ ] Benchmark CSV is generated on the demonstration machine.
- [ ] Hardware/software details are recorded with the benchmark results.
- [ ] README build/run instructions match the checked-out source.
- [ ] Demonstration script and sample input are included.

## Git release

From the project root, after final edits and verification. Initialize the repository only if it is not already present:

```bash
if [ ! -d .git ]; then git init -b main; fi
git status --short
git add CMakeLists.txt README.md include src tests benchmarks docs scripts examples .gitignore
git commit -m "Finalize Project 47 satellite tile scheduler"
git tag -a v1.0.0 -m "Project 47 final PBL release"
git status --short
git log -1 --oneline --decorate
```

Push only after checking the remote and repository visibility:

```bash
git remote -v
git push origin main
git push origin v1.0.0
```

The tag command creates a local release tag; this package does not assume a Git remote or publish anything automatically.
