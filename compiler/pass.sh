#!/bin/bash
set -e

# --- Input validation ---
if [ -z "$1" ]; then
    echo "Usage: $0 <source_file>"
    exit 1
fi

LLVM_DIR="llvm-project"
SOURCE="src/$1"
BASENAME=$(basename "${SOURCE%.*}")  # e.g. src/test.c -> test
IR_FILE="ir/${BASENAME}.ll"

mkdir -p ir

# --- Step 1: Configure and build LLVM ---
if [ ! -d "$LLVM_DIR/build" ]; then
    echo "==> Configuring LLVM..."
    cmake -S "$LLVM_DIR/llvm" -B "$LLVM_DIR/build" -G Ninja -DCMAKE_BUILD_TYPE=Debug
else
    echo "==> Build directory exists, skipping cmake configure..."
fi

echo "==> Building LLVM..."
ninja -C "$LLVM_DIR/build" opt llc

# --- Step 2: Compile source to LLVM IR ---
echo "==> Compiling $SOURCE to IR..."
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm "$SOURCE" -o "$IR_FILE"

# --- Step 3: Run taint pass, emitting annotated IR ---
TAINTED_IR="ir/${BASENAME}.tainted.ll"
echo "==> Running SecretTaint pass..."
"$LLVM_DIR/build/bin/opt" -S "$IR_FILE" -passes=secrettaint -o "$TAINTED_IR"
echo "==> Annotated IR written to $TAINTED_IR"

# --- Step 4: Run branch elimination pass ---
ELIM_IR="ir/${BASENAME}.elim.ll"
echo "==> Running SecretBranchElim pass..."
"$LLVM_DIR/build/bin/opt" -S "$TAINTED_IR" -passes=secretbranchelim -o "$ELIM_IR"
echo "==> Branch-eliminated IR written to $ELIM_IR"

# --- Step 5: Annotate secret values with register class marker ---
REGCLASS_IR="ir/${BASENAME}.regclass.ll"
echo "==> Running SecretRegClass pass..."
"$LLVM_DIR/build/bin/opt" -S "$ELIM_IR" -passes=secretregclass -o "$REGCLASS_IR"
echo "==> Register-class-annotated IR written to $REGCLASS_IR"

# --- Step 6: Compile to RISC-V assembly ---
ASM_FILE="ir/${BASENAME}.s"
echo "==> Compiling to RISC-V assembly..."
"$LLVM_DIR/build/bin/llc" -mtriple=riscv64 -mattr=+m,+f,+d \
    "$REGCLASS_IR" -o "$ASM_FILE"
echo "==> RISC-V assembly written to $ASM_FILE"

echo "==> Done."
