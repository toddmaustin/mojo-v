# Mojo-V Compiler

LLVM-based compiler from C/C++ to Mojo-V RISC-V assembly. Mojo-V is a RISC-V
ISA extension for secret computation. It partitions registers into secret
(x16–x31, f16–f31) and public (x0–x15, f0–f15) groups. When a secret register
value is stored to memory, the hardware automatically encrypts it; when an
encrypted value is loaded into a secret register, the hardware decrypts it.
The encryption key and mode (fast / strong / proof-carrying) are supplied by a
hardware data contract — the compiler is not involved in key management or
encryption. All security properties are enforced by hardware at decode time.

The compiler's responsibility is narrower: track which values are
secret-derived, make sure those values live in secret registers during
execution, and emit the correct hardware instructions for secret memory access.
The hardware then enforces the rest.

## Pipeline Overview

```
C/C++ source
     │
     ▼ clang
LLVM IR  (-O0, disable-O0-optnone)
     │
     ▼ SecretTaint pass  [implemented]
LLVM IR with !secret metadata
     │
     ▼ SecretBranchElim pass  [next]
LLVM IR with secret branches replaced by select
     │
     ▼ RISC-V instruction selection
MachineIR
     │
     ▼ Register allocation with SecretGPR class  [next]
MachineIR with physical registers assigned (secret values → x16–x31)
     │
     ▼ Post-RA encrypted store/load substitution  [next]
Mojo-V RISC-V assembly
```

---

## Step 1 — Secret Annotation (done)

Programmers mark secret values with the `SECRET` macro:

```c
#define SECRET __attribute__((annotate("secret")))

uint8_t SECRET key = 0x37;
```

Clang lowers this to an `llvm.var.annotation` intrinsic in the IR, which the
taint pass uses as a seed.

---

## Step 2 — SecretTaint Pass (done)

**Location:** `llvm/lib/Transforms/Utils/SecretTaint.cpp`  
**Pass name:** `secrettaint` (module pass)  
**Tests:** `test/secrettaint/`

Propagates `!secret` metadata through the IR using a two-level fixed-point:

**Intraprocedural** (within a function):
- Seeds: annotated pointers (`TaintedPtrs`) and secret-marked parameters (`TaintedVals`).
- Store rule: if the value or the pointer is tainted, tag the store and add the pointer to `TaintedPtrs`.
- Load rule: loading from a tainted pointer produces a tainted value.
- General rule: any instruction with a tainted operand is tainted.

**Interprocedural** (across functions, bottom-up call order):
- A call whose argument is tainted taints the call result (conservative: any secret input → secret output).
- When a callee is found to return a secret, all call sites in the module are tagged.
- Iterates until no new taint propagates.

**Pointer taint model:** taint attaches to the alloca, not to individual stores.
Once an alloca receives a secret value on any path, ALL stores and loads to it
are tagged — including ones that appear before the secret store in program order,
because the fixed-point loop re-visits the whole function body each iteration.
This is over-approximate by design.

---

## Step 3 — SecretBranchElim Pass (next)

Branching on a secret value leaks information through control-flow timing.
Any `br` whose condition carries `!secret` metadata must be eliminated.

**Strategy:**
1. Find `br i1 %cond` instructions where `%cond` is `!secret`.
2. Convert the enclosing if/else into a `select` (the LLVM equivalent of a
   conditional move). This requires that both branches are free of
   non-speculatable side effects (stores, calls, etc.).
3. If the transformation is impossible — e.g., the branches contain side
   effects that cannot be hoisted or speculated — emit a compiler error
   identifying the offending source location.

This pass runs on IR, before instruction selection.

---

## Step 4 — Register Allocation: SecretGPR Class (next)

Mojo-V partitions the 32 RISC-V integer registers into:
- **Public:** x0–x15
- **Secret:** x16–x31 (note: must not use x16 or below for compressed-instruction safety; see project `NOTES.txt`)

Any value carrying `!secret` taint must be allocated to a secret physical
register so the hardware can apply its encryption/decryption automatically.

**Implementation plan:**
1. Define a `SecretGPR` register class in `RISCVRegisterInfo.td` covering x16–x31.
2. After instruction selection, add a MachineIR pass that walks virtual
   registers, checks whether the defining instruction carries `!secret`
   metadata, and constrains those virtual registers to the `SecretGPR` class.
3. The standard LLVM register allocator then respects the class constraint.

---

## Step 5 — Encrypted Store/Load Substitution (next)

After register allocation, physical registers are known. A post-RA pass
replaces conventional load/store instructions with Mojo-V's encrypted variants
whenever the source or destination register is in the secret range (x16–x31).

The Mojo-V ISA adds only four new instructions in total; the encrypted load and
store are two of them. Once emitted, these instructions trigger hardware
encryption (on store) and decryption (on load) using the key and mode
established by the data contract loaded into the CPU. The compiler does not
select or implement an encryption algorithm — it only ensures the correct
instruction opcodes are emitted.

---

## Building and Testing

```bash
# Build LLVM and run the taint pass on a source file
./pass.sh test.c          # output: ir/test.tainted.ll

# Run the taint-pass test suite
./test/run_tests.sh
```
