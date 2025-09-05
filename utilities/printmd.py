#!/usr/bin/env python3
import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: printmd.py FILE [...]", file=sys.stderr)
        sys.exit(1)

    for path in sys.argv[1:]:
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

