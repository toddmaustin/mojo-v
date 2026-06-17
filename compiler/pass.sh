#!/bin/bash
set -e

# --- Input validation ---
if [ -z "$1" ]; then
    echo "Usage: $0 <source_file>"
    exit 1
fi

LLVM_DIR="llvm-project"

# Parse optional --target=<host|spike> flag (env var MOJOV_TARGET also works)
TARGET="${MOJOV_TARGET:-host}"
for arg in "$@"; do
    case "$arg" in
        --target=*) TARGET="${arg#--target=}" ;;
    esac
done
# Strip the flag from positional args so SOURCE parsing still works
set -- $(echo "$@" | sed 's/--target=[^ ]*//g')

# Accept either a bare filename (looked up in src/) or a full/relative path.
if [[ "$1" = /* ]] || [[ "$1" = ./* ]] || [[ "$1" = */* ]]; then
    SOURCE="$1"
else
    SOURCE="src/$1"
fi
BASENAME=$(basename "${SOURCE%.*}")  # e.g. src/test.c -> test
IR_FILE="ir/${BASENAME}.ll"

LIBMIN_DIR="../bringup-bench/common"
LIBTARG_DIR="../bringup-bench/target"
EXO_DIR="../exo"

case "$TARGET" in
    spike) TARGET_DEFINES="-DTARGET_SPIKE -DLIBMIN_MALLOC_ALIGN_BYTES=8" ;;
    host|*) TARGET_DEFINES="-DTARGET_HOST" ;;
esac

LIBMIN_CFLAGS=(-O0 -Xclang -disable-O0-optnone -S -emit-llvm
               $TARGET_DEFINES -D_SSIZE_T
               -I "$LIBTARG_DIR" -I "$LIBMIN_DIR" -I "$EXO_DIR")

mkdir -p ir ir/libmin

# --- Step 1: Configure and build LLVM ---
if [ ! -d "$LLVM_DIR/build" ]; then
    echo "==> Configuring LLVM..."
    cmake -S "$LLVM_DIR/llvm" -B "$LLVM_DIR/build" -G Ninja -DCMAKE_BUILD_TYPE=Debug
else
    echo "==> Build directory exists, skipping cmake configure..."
fi

echo "==> Building LLVM..."
ninja -C "$LLVM_DIR/build" opt llc llvm-link

# --- Step 2: Compile source to LLVM IR ---
# Use clang++ for C++ sources (.cc/.cpp), clang for C sources.
echo "==> Compiling $SOURCE to IR..."
case "$SOURCE" in
    *.cc|*.cpp) COMPILER="clang++ -target riscv64-unknown-elf" ;;
    *)          COMPILER="clang" ;;
esac
$COMPILER "${LIBMIN_CFLAGS[@]}" "$SOURCE" -o "$IR_FILE"

# --- Step 2b: Compile libmin + libtarg to IR and link into one module ---
echo "==> Compiling libmin sources to IR..."
libmin_irs=()
for src in "$LIBMIN_DIR"/libmin_*.c; do
    name=$(basename "${src%.c}")
    ir="ir/libmin/${name}.ll"
    clang "${LIBMIN_CFLAGS[@]}" "$src" -o "$ir" 2>/dev/null
    libmin_irs+=("$ir")
done

echo "==> Compiling libtarg, simon, and mojov-utils to IR..."
clang   "${LIBMIN_CFLAGS[@]}" "$LIBTARG_DIR/libtarg.c"      -o "ir/libmin/libtarg.ll"     2>/dev/null
# simon.c and mojov-utils.c use C++ types (bool, etc.) — compile as C++.
clang++ "${LIBMIN_CFLAGS[@]}" "$LIBMIN_DIR/simon.c"         -o "ir/libmin/simon.ll"       2>/dev/null
clang++ "${LIBMIN_CFLAGS[@]}" "$LIBTARG_DIR/mojov-utils.c"  -o "ir/libmin/mojov-utils.ll" 2>/dev/null

echo "==> Linking with libmin + exo support..."
LINKED_IR="ir/${BASENAME}.linked.ll"
"$LLVM_DIR/build/bin/llvm-link" -S \
    "$IR_FILE" "${libmin_irs[@]}" \
    "ir/libmin/libtarg.ll" "ir/libmin/simon.ll" "ir/libmin/mojov-utils.ll" \
    -o "$LINKED_IR"
echo "==> Linked IR written to $LINKED_IR"

# --- Step 3: Run taint pass, emitting annotated IR ---
TAINTED_IR="ir/${BASENAME}.tainted.ll"
echo "==> Running SecretTaint pass..."
"$LLVM_DIR/build/bin/opt" -S "$LINKED_IR" -passes=secrettaint -o "$TAINTED_IR"
echo "==> Annotated IR written to $TAINTED_IR"

# --- Step 4: Run branch elimination pass ---
# mem2reg (function pass) must run before SecretBranchElim (module pass) so
# that alloca/store/load patterns become phi nodes that the pass can fold.
MEM2REG_IR="ir/${BASENAME}.mem2reg.ll"
echo "==> Running mem2reg to promote allocas to SSA..."
"$LLVM_DIR/build/bin/opt" -S "$TAINTED_IR" -passes="function(mem2reg)" -o "$MEM2REG_IR"

ELIM_IR="ir/${BASENAME}.elim.ll"
echo "==> Running SecretBranchElim pass..."
"$LLVM_DIR/build/bin/opt" -S "$MEM2REG_IR" -passes=secretbranchelim -o "$ELIM_IR"
echo "==> Branch-eliminated IR written to $ELIM_IR"

# --- Step 5: Annotate secret values with register class marker ---
REGCLASS_IR="ir/${BASENAME}.regclass.ll"
echo "==> Running SecretRegClass pass..."
"$LLVM_DIR/build/bin/opt" -S "$ELIM_IR" -passes=secretregclass -o "$REGCLASS_IR"
echo "==> Register-class-annotated IR written to $REGCLASS_IR"

# --- Step 6: Strip host CPU attributes so llc can target RISC-V ---
# System clang (Apple M1) embeds host-specific "target-cpu" and
# "target-features" attributes in each function. The RISC-V llc backend
# rejects them, so we strip them here before code generation.
CLEAN_IR="ir/${BASENAME}.clean.ll"
python3 - "$REGCLASS_IR" "$CLEAN_IR" << 'PYEOF'
import re, sys
ir = open(sys.argv[1]).read()
ir = re.sub(r' "target-cpu"="[^"]*"', '', ir)
ir = re.sub(r' "target-features"="[^"]*"', '', ir)
open(sys.argv[2], 'w').write(ir)
PYEOF
echo "==> Host CPU attributes stripped."

# --- Step 7: Compile to RISC-V assembly ---
ASM_FILE="ir/${BASENAME}.s"
echo "==> Compiling to RISC-V assembly..."
"$LLVM_DIR/build/bin/llc" -mtriple=riscv64 -mattr=+m,+f,+d \
    "$CLEAN_IR" -o "$ASM_FILE"
echo "==> RISC-V assembly written to $ASM_FILE"

if [ "$TARGET" = "spike" ]; then
    OBJ_FILE="ir/${BASENAME}.o"
    echo "==> Generating RISC-V object file (spike target)..."
    "$LLVM_DIR/build/bin/llc" -mtriple=riscv64 -mattr=+m,+f,+d \
        -filetype=obj "$CLEAN_IR" -o "$OBJ_FILE"
    echo "==> Object file written to $OBJ_FILE"
fi

echo "==> Done."
