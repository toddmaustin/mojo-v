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

run_test() {
    local test_file="$1"
    local pass_name="$2"
    local expect_error="$3"   # "1" if the test expects opt to fail
    local test_name
    test_name="$(basename "$test_file")"

    if [ "$expect_error" = "1" ]; then
        # Capture stderr; the test passes if FileCheck finds the expected error.
        if "$OPT" -S -passes="$pass_name" "$test_file" 2>&1 >/dev/null \
                | "$FILECHECK" "$test_file" 2>&1; then
            echo "PASS: $test_name"
            PASS_COUNT=$((PASS_COUNT + 1))
        else
            echo "FAIL: $test_name"
            FAIL_COUNT=$((FAIL_COUNT + 1))
        fi
    else
        if "$OPT" -S -passes="$pass_name" "$test_file" 2>/dev/null \
                | "$FILECHECK" "$test_file" 2>&1; then
            echo "PASS: $test_name"
            PASS_COUNT=$((PASS_COUNT + 1))
        else
            echo "FAIL: $test_name"
            FAIL_COUNT=$((FAIL_COUNT + 1))
        fi
    fi
}

echo "==> SecretTaint tests"
for f in "$SCRIPT_DIR/secrettaint/"*.ll; do
    run_test "$f" "secrettaint" "0"
done

echo ""
echo "==> SecretBranchElim tests"
for f in "$SCRIPT_DIR/secretbranchelim/"*.ll; do
    # Tests using 'not opt ...' in their RUN line expect an error.
    if grep -q "^; RUN: not " "$f"; then
        run_test "$f" "secretbranchelim" "1"
    else
        run_test "$f" "secretbranchelim" "0"
    fi
done

echo ""
echo "Results: $PASS_COUNT passed, $FAIL_COUNT failed"
[ "$FAIL_COUNT" -eq 0 ]
