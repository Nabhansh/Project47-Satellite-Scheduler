#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PARENT="$(dirname "$ROOT")"
NAME="$(basename "$ROOT")"
OUTPUT="${1:-$PARENT/${NAME}_source_submission.zip}"

python3 - "$ROOT" "$OUTPUT" <<'PYCODE'
from pathlib import Path
import sys
import zipfile

root = Path(sys.argv[1]).resolve()
output = Path(sys.argv[2]).resolve()
output.parent.mkdir(parents=True, exist_ok=True)

excluded_directories = {".git", "outputs", "build", "__pycache__"}

def excluded(relative: Path) -> bool:
    return any(
        part in excluded_directories or part.startswith("build-")
        for part in relative.parts[:-1]
    )

with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        if excluded(relative):
            continue
        archive.write(path, Path(root.name) / relative)

with zipfile.ZipFile(output) as archive:
    names = archive.namelist()
    forbidden = [name for name in names if any(
        part in excluded_directories or part.startswith("build-")
        for part in Path(name).parts
    )]
    if forbidden:
        raise SystemExit(f"Refusing to create package containing generated artifacts: {forbidden[:5]}")
    print(f"Created source-only package: {output}")
    print(f"Included files: {len(names)}")
PYCODE
