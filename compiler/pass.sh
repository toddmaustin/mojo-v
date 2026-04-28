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
ninja -C "$LLVM_DIR/build"

# --- Step 2: Build opt ---
echo "==> Building opt..."
ninja -C "$LLVM_DIR/build" opt

# --- Step 3: Compile source to LLVM IR ---
echo "==> Compiling $SOURCE to IR..."
clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm "$SOURCE" -o "$IR_FILE"

# --- Step 4: Run the taint pass, emitting annotated IR ---
TAINTED_IR="ir/${BASENAME}.tainted.ll"
echo "==> Running SecretTaint pass..."
"$LLVM_DIR/build/bin/opt" -S "$IR_FILE" -passes=secrettaint -o "$TAINTED_IR"

echo "==> Annotated IR written to $TAINTED_IR"
echo "==> Done."
