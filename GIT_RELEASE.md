# Git Repository and v1.0.0 Release Instructions

These commands are intended to be run on Fedora from the **top-level** `project47_hackathon_submission` directory. They preserve both timestamped phases in one repository and create a local annotated release tag.

## 1. Copy benchmark evidence first

From the Phase II project directory:

```bash
mkdir -p ../../evidence
cp outputs/benchmark_results.csv ../../evidence/fedora_benchmark_results_2026-11-19.csv
{
  echo "Capture date: 2026-11-19"
  echo
  echo "--- OS/kernel ---"
  uname -a
  echo
  echo "--- CPU ---"
  lscpu
  echo
  echo "--- RAM ---"
  free -h
  echo
  echo "--- C++ compiler ---"
  c++ --version
  echo
  echo "--- CMake ---"
  cmake --version
} > ../../evidence/fedora_machine_metadata_2026-11-19.txt
```

Check that both evidence files exist:

```bash
ls -lh ../../evidence/
```

## 2. Initialize the repository at the submission root

```bash
cd ~/Music/project47_hackathon_submission

git init
git branch -M main
```

If Git has no configured identity, set your real author details (do not use placeholder values):

```bash
git config user.name "Your Name"
git config user.email "your-verified-email@example.com"
```

## 3. Review and commit

```bash
git add -A
git status --short
git diff --cached --stat
git commit -m "Project 47: finalize Phase I and Phase II submission"
```

Inspect the commit:

```bash
git status --short
git show --stat --oneline HEAD
```

The working tree should be clean after the commit. Build directories, generated `outputs/`, and binaries are ignored by `.gitignore` files.

## 4. Create the annotated release tag

```bash
git tag -a v1.0.0 -m "Project 47 Phase II verified submission"
git tag -n
git show --no-patch --decorate v1.0.0
```

The tag is local until pushed to a remote. If a Git remote has been created and the repository should be published:

```bash
git remote -v
git push -u origin main --tags
```

Do not run the push command until `origin` points to the intended repository and its access/visibility settings are correct.

## 5. Verify the final tracked content

```bash
git ls-files | grep -E '(^|/)(build|build-release|build-strict|outputs)/' 
```

This command should print nothing. Confirm both snapshots are tracked:

```bash
git ls-files | grep 'Phase_I_20261118_1636/' | head
git ls-files | grep 'Phase_II_20261119_1148/' | head
```

Keep the source-only ZIP as the hand-in archive. Do not include `.git/`, build directories, generated output directories, or compiled executables inside that ZIP.
