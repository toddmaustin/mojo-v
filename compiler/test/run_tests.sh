#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
OPT="$REPO_ROOT/llvm-project/build/bin/opt"
FILECHECK="$REPO_ROOT/llvm-project/build/bin/FileCheck"

if [ ! -f "$OPT" ]; then
    echo "Error: opt not found at $OPT — run pass.sh first to build LLVM." >&2
    exit 1
fi

if [ ! -f "$FILECHECK" ]; then
    echo "Error: FileCheck not found at $FILECHECK." >&2
    exit 1
fi

PASS_COUNT=0
FAIL_COUNT=0

for test_file in "$SCRIPT_DIR/secrettaint/"*.ll; do
    test_name="$(basename "$test_file")"
    if "$OPT" -S -passes=secrettaint "$test_file" 2>/dev/null \
            | "$FILECHECK" "$test_file" 2>&1; then
        echo "PASS: $test_name"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "FAIL: $test_name"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
done

echo ""
echo "Results: $PASS_COUNT passed, $FAIL_COUNT failed"
[ "$FAIL_COUNT" -eq 0 ]
