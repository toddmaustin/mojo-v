#!/bin/bash
#
# run_spike.sh — compile a Mojo-V C source through the LLVM compiler and
# run the resulting ELF on the Spike RISC-V ISA simulator.
#
# Usage:
#   ./run_spike.sh <source>
#
# <source> can be:
#   - a filename in compiler/src/  (e.g. spike_demo.c)
#   - a path relative to the repo root (e.g. compiler/src/spike_demo.c)
#   - an absolute path
#
# Example:
#   ./run_spike.sh spike_demo.c
#   ./run_spike.sh compiler/src/spike_demo.c
#
# Prerequisites (see repo NOTES.txt for build instructions):
#   1. RISC-V GNU toolchain at /opt/riscv/
#        brew tap riscv-software-src/riscv
#        brew install riscv-gnu-toolchain
#        sudo ln -sf $(brew --prefix)/opt/riscv-gnu-toolchain /opt/riscv
#
#   2. Spike simulator:
#        mkdir -p riscv-isa-sim/build
#        (cd riscv-isa-sim/build && ../configure --prefix=/opt/riscv)
#        make -C riscv-isa-sim/build -j$(nproc)
#
#   3. Spike MMIO plugin:
#        make -C bringup-bench/target build
#
set -e

REPO="$(cd "$(dirname "$0")" && pwd)"
COMPILER="$REPO/compiler"
BB_TARGET="$REPO/bringup-bench/target"
SPIKE="$REPO/riscv-isa-sim/build/spike"
SPIKE_PLUGIN="$BB_TARGET/spike_mmio_plugin.so"
RISCV_GCC="/opt/riscv/bin/riscv64-unknown-elf-gcc"
RISCV_LIBGCC_ROOT="/opt/riscv/lib/gcc/riscv64-unknown-elf"

# ---------------------------------------------------------------------------
# 1. Check prerequisites
# ---------------------------------------------------------------------------
fail() { echo "ERROR: $*" >&2; exit 1; }

[ -n "$1" ] || { echo "Usage: $0 <source_file>"; exit 1; }

[ -x "$RISCV_GCC" ] || fail "RISC-V GCC not found at $RISCV_GCC
  Install the toolchain (this takes ~20 min to compile from source):
    brew tap riscv-software-src/riscv
    brew install riscv-gnu-toolchain
    sudo ln -sf \$(brew --prefix)/opt/riscv-gnu-toolchain /opt/riscv"

[ -x "$SPIKE" ] || fail "Spike not built. Run:
    brew install dtc          # device-tree-compiler, needed by Spike
    mkdir -p riscv-isa-sim/build
    (cd riscv-isa-sim/build && ../configure --prefix=/opt/riscv)
    make -C riscv-isa-sim/build -j\$(nproc)"

[ -f "$SPIKE_PLUGIN" ] || fail "Spike MMIO plugin not built. Run:
    make -C bringup-bench/target build"

# ---------------------------------------------------------------------------
# 2. Resolve source path
# ---------------------------------------------------------------------------
INPUT="$1"
if [[ "$INPUT" = /* ]]; then
    SRC_FULL="$INPUT"                          # absolute path
elif [[ "$INPUT" = */* ]]; then
    # relative path: strip leading "compiler/" if present
    INPUT="${INPUT#compiler/}"
    SRC_FULL="$COMPILER/$INPUT"
else
    SRC_FULL="$COMPILER/src/$INPUT"           # bare filename → compiler/src/
fi
[ -f "$SRC_FULL" ] || fail "Source file not found: $SRC_FULL"

BASENAME=$(basename "${SRC_FULL%.*}")
BUILD_DIR="$COMPILER/ir/spike_${BASENAME}"
mkdir -p "$BUILD_DIR"

# ---------------------------------------------------------------------------
# 3. Compiler pipeline (TARGET_SPIKE) → .o
# ---------------------------------------------------------------------------
echo "==> Compiling $SRC_FULL through the Mojo-V LLVM pipeline (spike mode)..."
(cd "$COMPILER" && MOJOV_TARGET=spike bash pass.sh "$SRC_FULL")

OBJ="$COMPILER/ir/${BASENAME}.o"
[ -f "$OBJ" ] || fail "Expected object file not found: $OBJ"

# ---------------------------------------------------------------------------
# 4. Find libgcc.a
#    Prefer the default (lp64d) build; fall back to any available.
# ---------------------------------------------------------------------------
LIBGCC=$(find "$RISCV_LIBGCC_ROOT" -maxdepth 2 -name "libgcc.a" 2>/dev/null | \
         grep -v "/rv32\|/rv64imac\b" | head -1)
[ -n "$LIBGCC" ] || \
    LIBGCC=$(find "$RISCV_LIBGCC_ROOT" -name "libgcc.a" 2>/dev/null | head -1)
[ -n "$LIBGCC" ] || fail "libgcc.a not found under $RISCV_LIBGCC_ROOT"
echo "==> Using libgcc: $LIBGCC"

# ---------------------------------------------------------------------------
# 5. Link: compiler .o + spike-crt0.S → ELF
#    GCC acts as driver to assemble crt0.S and link everything.
# ---------------------------------------------------------------------------
ELF="$BUILD_DIR/${BASENAME}.elf"
echo "==> Linking → $ELF ..."
"$RISCV_GCC" \
    -static \
    -march=rv64gc \
    -mabi=lp64d \
    -fvisibility=hidden \
    -nostdlib \
    -ffreestanding \
    -mcmodel=medany \
    -nostartfiles \
    -T "$BB_TARGET/spike-map.ld" \
    "$OBJ" \
    "$BB_TARGET/spike-crt0.S" \
    -o "$ELF" \
    "$LIBGCC"
echo "==> ELF: $ELF"

# ---------------------------------------------------------------------------
# 6. Run on Spike
# ---------------------------------------------------------------------------
echo "==> Running on Spike..."
echo "------------------------------------------------------------"
"$SPIKE" \
    --mojov-pk="${MOJOV_PK:-$BB_TARGET/pk-file.pem}" \
    --mojov-sk="${MOJOV_SK:-$BB_TARGET/sk-file.pem}" \
    --isa=rv64gc_zicond_zkmojov_zicntr \
    --misaligned \
    --extlib="$SPIKE_PLUGIN" \
    -m0x100000:0x820000 \
    --device=spike_mmio_plugin,0x20000 \
    "$ELF"
