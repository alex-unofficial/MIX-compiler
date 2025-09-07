#!/usr/bin/env python3
import sys
import os
from pathlib import Path

try:
    import pathspec
except ImportError:
    print("You need to install pathspec: pip install pathspec", file=sys.stderr)
    sys.exit(1)

def load_gitignore(base_dir="."):
    gitignore_path = Path(base_dir) / ".gitignore"
    if gitignore_path.exists():
        with open(gitignore_path, "r", encoding="utf-8") as f:
            return pathspec.PathSpec.from_lines("gitwildmatch", f)
    return pathspec.PathSpec.from_lines("gitwildmatch", [])

def main():
    if len(sys.argv) < 2:
        print("Usage: printmd.py FILE [...]", file=sys.stderr)
        sys.exit(1)

    spec = load_gitignore(".")

    for path in sys.argv[1:]:
        rel_path = os.path.relpath(path, ".")
        if spec.match_file(rel_path):
            continue  # skip ignored files

        print(f"file `{path}`:")
        print("```")
        try:
            with open(path, "r", encoding="utf-8", errors="replace") as f:
                content = f.read()
                sys.stdout.write(content)
                if not content.endswith("\n"):
                    print()
        except Exception as e:
            print(f"[error opening file: {e}]", file=sys.stderr)
        print("```")
        print()

if __name__ == "__main__":
    main()

