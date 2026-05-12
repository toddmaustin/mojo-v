#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
OPT="$REPO_ROOT/llvm-project/build/bin/opt"
LLC="$REPO_ROOT/llvm-project/build/bin/llc"
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

run_opt_test() {
    local test_file="$1"
    local pass_name="$2"
    local expect_error="$3"   # "1" if the test expects opt to fail
    local test_name
    test_name="$(basename "$test_file")"

    if [ "$expect_error" = "1" ]; then
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

run_llc_test() {
    local test_file="$1"
    local test_name
    test_name="$(basename "$test_file")"

    if "$LLC" -mtriple=riscv64 -mattr=+m,+f,+d "$test_file" -o - 2>/dev/null \
            | "$FILECHECK" "$test_file" 2>&1; then
        echo "PASS: $test_name"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "FAIL: $test_name"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

echo "==> SecretTaint tests"
for f in "$SCRIPT_DIR/secrettaint/"*.ll; do
    run_opt_test "$f" "secrettaint" "0"
done

echo ""
echo "==> SecretBranchElim tests"
for f in "$SCRIPT_DIR/secretbranchelim/"*.ll; do
    if grep -q "^; RUN: not " "$f"; then
        run_opt_test "$f" "secretbranchelim" "1"
    else
        run_opt_test "$f" "secretbranchelim" "0"
    fi
done

echo ""
echo "==> SecretRegClass tests"
for f in "$SCRIPT_DIR/secretregclass/"*.ll; do
    if grep -q "^; RUN:.*llc" "$f"; then
        run_llc_test "$f"
    else
        run_opt_test "$f" "secretregclass" "0"
    fi
done

echo ""
echo "==> End-to-end tests"
if [ -f "$SCRIPT_DIR/e2e/run_tests.sh" ]; then
    # Run in a subshell; capture its exit status without triggering set -e.
    e2e_output="$("$SCRIPT_DIR/e2e/run_tests.sh" 2>&1)" || true
    e2e_pass=$(echo "$e2e_output" | grep -c "^PASS:" || true)
    e2e_fail=$(echo "$e2e_output" | grep -c "^FAIL" || true)
    echo "$e2e_output" | grep -E "^(PASS|FAIL)"
    PASS_COUNT=$((PASS_COUNT + e2e_pass))
    FAIL_COUNT=$((FAIL_COUNT + e2e_fail))
fi

echo ""
echo "Results: $PASS_COUNT passed, $FAIL_COUNT failed"
[ "$FAIL_COUNT" -eq 0 ]
