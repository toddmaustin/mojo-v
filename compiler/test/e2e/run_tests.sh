#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
OPT="$REPO_ROOT/llvm-project/build/bin/opt"
LLC="$REPO_ROOT/llvm-project/build/bin/llc"
FILECHECK="$REPO_ROOT/llvm-project/build/bin/FileCheck"

PASS_COUNT=0
FAIL_COUNT=0

die() { echo "Error: $*" >&2; exit 1; }
[ -f "$OPT" ]       || die "opt not found — run pass.sh to build LLVM"
[ -f "$LLC" ]       || die "llc not found"
[ -f "$FILECHECK" ] || die "FileCheck not found"

has_prefix() {
    local src="$1" prefix="$2"
    grep -qE "^// ${prefix}(-[A-Z]+)?:" "$src"
}

strip_attrs() {
    sed -E 's/ "target-cpu"="[^"]*"//g; s/ "target-features"="[^"]*"//g' "$1"
}

run_e2e() {
    local src="$1"
    local base
    base="$(basename "${src%.c}")"
    local tmpdir
    tmpdir="$(mktemp -d)"
    trap 'rm -rf "$tmpdir"' RETURN

    local raw_ir="$tmpdir/raw.ll"
    local tainted_ir="$tmpdir/tainted.ll"
    local elim_ir="$tmpdir/elim.ll"
    local regclass_ir="$tmpdir/regclass.ll"
    local clean_ir="$tmpdir/clean.ll"
    local asm="$tmpdir/out.s"

    # Compile C → IR (suppress host attributes from system clang)
    if ! clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm \
            "$src" -o "$raw_ir" 2>/dev/null; then
        echo "FAIL [compile]: $base"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return
    fi

    # SecretTaint
    if ! "$OPT" -S -passes=secrettaint "$raw_ir" -o "$tainted_ir" 2>/dev/null; then
        echo "FAIL [secrettaint]: $base"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return
    fi

    # mem2reg (function pass) must run separately before the module-level
    # SecretBranchElim so that alloca/store/load patterns become phi nodes.
    local mem2reg_ir="$tmpdir/mem2reg.ll"
    if ! "$OPT" -S -passes="function(mem2reg)" "$tainted_ir" -o "$mem2reg_ir" 2>/dev/null; then
        echo "FAIL [mem2reg]: $base"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return
    fi

    # SecretBranchElim — check first whether this test expects a compiler error.
    if grep -q "^// ELIM-ERROR:" "$src"; then
        # The test expects secretbranchelim to fail. Capture stderr and check
        # it against the ELIM-ERROR patterns; treat pipeline success as a failure.
        local elim_err
        elim_err="$("$OPT" -S -passes=secretbranchelim "$mem2reg_ir" \
                        -o "$elim_ir" 2>&1 >/dev/null)" || true
        if "$OPT" -S -passes=secretbranchelim "$mem2reg_ir" \
                -o /dev/null 2>/dev/null; then
            echo "FAIL [ELIM-ERROR: pass succeeded unexpectedly]: $base"
            FAIL_COUNT=$((FAIL_COUNT + 1))
        elif echo "$elim_err" | "$FILECHECK" --check-prefix=ELIM-ERROR "$src" \
                2>/dev/null; then
            echo "PASS: $base"
            PASS_COUNT=$((PASS_COUNT + 1))
        else
            echo "FAIL [ELIM-ERROR: wrong error message]: $base"
            FAIL_COUNT=$((FAIL_COUNT + 1))
        fi
        return
    fi

    if ! "$OPT" -S -passes=secretbranchelim "$mem2reg_ir" -o "$elim_ir" 2>/dev/null; then
        echo "FAIL [secretbranchelim]: $base"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return
    fi

    # SecretRegClass
    if ! "$OPT" -S -passes=secretregclass "$elim_ir" -o "$regclass_ir" 2>/dev/null; then
        echo "FAIL [secretregclass]: $base"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return
    fi

    # Strip host CPU attributes so RISC-V llc accepts the IR
    strip_attrs "$regclass_ir" > "$clean_ir"

    # Assemble to RISC-V
    if ! "$LLC" -mtriple=riscv64 -mattr=+m,+f,+d "$clean_ir" -o "$asm" 2>/dev/null; then
        echo "FAIL [llc]: $base"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return
    fi

    local ok=1

    # FileCheck TAINT prefix against tainted IR
    if has_prefix "$src" TAINT; then
        if ! "$FILECHECK" --check-prefix=TAINT "$src" < "$tainted_ir" 2>/dev/null; then
            echo "FAIL [TAINT]: $base"
            FAIL_COUNT=$((FAIL_COUNT + 1))
            ok=0
        fi
    fi

    # FileCheck ELIM prefix against elim IR
    if has_prefix "$src" ELIM; then
        if ! "$FILECHECK" --check-prefix=ELIM "$src" < "$elim_ir" 2>/dev/null; then
            echo "FAIL [ELIM]: $base"
            FAIL_COUNT=$((FAIL_COUNT + 1))
            ok=0
        fi
    fi

    # FileCheck REGCLASS prefix against regclass IR
    if has_prefix "$src" REGCLASS; then
        if ! "$FILECHECK" --check-prefix=REGCLASS "$src" < "$regclass_ir" 2>/dev/null; then
            echo "FAIL [REGCLASS]: $base"
            FAIL_COUNT=$((FAIL_COUNT + 1))
            ok=0
        fi
    fi

    # FileCheck ASM prefix against assembly
    if has_prefix "$src" ASM; then
        if ! "$FILECHECK" --check-prefix=ASM "$src" < "$asm" 2>/dev/null; then
            echo "FAIL [ASM]: $base"
            FAIL_COUNT=$((FAIL_COUNT + 1))
            ok=0
        fi
    fi

    if [ "$ok" = "1" ]; then
        echo "PASS: $base"
        PASS_COUNT=$((PASS_COUNT + 1))
    fi
}

echo "==> End-to-end tests"
for f in "$SCRIPT_DIR/src/"*.c; do
    run_e2e "$f"
done

echo ""
echo "Results: $PASS_COUNT passed, $FAIL_COUNT failed"
[ "$FAIL_COUNT" -eq 0 ]
