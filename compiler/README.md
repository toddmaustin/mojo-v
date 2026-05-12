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
     ▼ mem2reg + SecretBranchElim pass  [implemented]
LLVM IR with secret branches replaced by select
     │
     ▼ SecretRegClass pass  [implemented]
LLVM IR with @llvm.riscv.mojov.secret wrappers on !secret values
     │
     ▼ RISC-V instruction selection
     ▼ SecretConstraint pre-RA pass  [implemented]
MachineIR: all defs that read from SecretGPR constrained to SecretGPR
     │
     ▼ Register allocation
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

## Step 3 — SecretBranchElim Pass (done)

**Location:** `llvm/lib/Transforms/Utils/SecretBranchElim.cpp`  
**Pass name:** `secretbranchelim` (module pass)  
**Tests:** `test/secretbranchelim/`

Branching on a secret value leaks information through control-flow timing.
Any `br` whose condition carries `!secret` metadata must be eliminated.

**Strategy:**
1. Run `mem2reg` first to promote alloca/store/load patterns to SSA phi nodes,
   which SecretBranchElim requires to detect diamond-shaped control flow.
2. Find `br i1 %cond` instructions where `%cond` is `!secret`.
3. Convert the enclosing if/else into a `select` (the LLVM equivalent of a
   conditional move). This requires that both branches form a diamond shape
   and contain only speculatable instructions.
4. If the transformation is impossible — e.g., the branches contain side
   effects (stores, calls, etc.) — emit a compiler error identifying the
   offending source location.

On RISC-V, `select` lowers to `czero.eqz`/`czero.nez` from the Zicond
extension, which execute both sides and pick the result without branching.

---

## Step 4 — SecretRegClass Pass (done)

**Location:** `llvm/lib/Transforms/Utils/SecretRegClass.cpp`  
**Pass name:** `secretregclass` (module pass)  
**Tests:** `test/secretregclass/`

Ensures that every `!secret`-tagged integer value is allocated to a register
in the secret range (x16–x31) during RISC-V code generation.

**IR-level pass:** walks each function and wraps every `!secret` integer
instruction with a call to `@llvm.riscv.mojov.secret.*`. Sub-word values
(i1/i8/i16/i32) are zero-extended to i64 before the call and truncated back
afterward, since SecretGPR only holds i64 on RV64:

```llvm
; Before:
%sum = add i64 %x, %y, !secret !0

; After:
%sum = add i64 %x, %y, !secret !0
%sum.secret = call i64 @llvm.riscv.mojov.secret.i64(i64 %sum)
```

All subsequent uses of `%sum` are replaced by `%sum.secret`.

**Backend wiring:**
- `SecretGPR` register class (x16–x31) defined in `RISCVRegisterInfo.td`.
  Allocation order: caller-saved first (a6–a7, t3–t6), then callee-saved (s2–s11).
- `@llvm.riscv.mojov.secret` intrinsic defined in `IntrinsicsRISCV.td`.
- `RISCVISelDAGToDAG.cpp` handles the intrinsic during instruction selection
  by emitting `COPY_TO_REGCLASS` into `SecretGPR`.

---

## Step 4b — SecretConstraint Pre-RA Pass (done)

**Location:** `llvm/lib/Target/RISCV/RISCVSecretConstraint.cpp`  
**Pass type:** pre-RA `MachineFunctionPass`

The Mojo-V hardware rule: **any instruction that reads from a secret register
must also write its result to a secret register.** Placing a secret-derived
value in a public register is a hardware fault.

The `COPY_TO_REGCLASS` emitted for the `@llvm.riscv.mojov.secret` intrinsic
only constrains the *intrinsic's* output virtual register. The arithmetic
instruction that feeds it (e.g., `xor`) can still be assigned a public GPR
destination, producing a forbidden pattern like `xor a0, a6, a0`.

This pass runs before register allocation and performs a forward fixed-point
walk over all machine instructions. For every instruction where any USE virtual
register belongs to `SecretGPR`, it tightens all DEF virtual registers to
`SecretGPR` (via `getCommonSubClass`). Floating-point or other disjoint classes
are left unchanged.

Result: the register allocator sees the SecretGPR constraint on the arithmetic
instruction itself, not just on the downstream copy, and allocates accordingly:

```asm
; Before this pass:
xor  a0, a6, a0   ; reads secret a6, writes public a0 — hardware fault

; After this pass:
xor  a6, a6, a0   ; reads secret a6, writes secret a6 — correct
```

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
# Build LLVM and run the full pipeline on a source file:
#   src/<file> → ir/<file>.tainted.ll → ir/<file>.mem2reg.ll
#               → ir/<file>.elim.ll → ir/<file>.regclass.ll
#               → ir/<file>.clean.ll → ir/<file>.s
./pass.sh <source_file>

# Run the full test suite (unit tests + end-to-end tests)
./test/run_tests.sh
```

### Test layout

| Directory | Type | What it tests |
|---|---|---|
| `test/secrettaint/` | opt FileCheck | SecretTaint pass on hand-crafted IR |
| `test/secretbranchelim/` | opt FileCheck | SecretBranchElim pass on hand-crafted IR |
| `test/secretregclass/` | opt / llc FileCheck | SecretRegClass IR wrapping + SecretGPR regalloc |
| `test/e2e/src/` | C source → full pipeline | Taint, branch elim, regclass, and assembly from real C |

End-to-end tests use per-prefix `FileCheck` patterns embedded in C comments
(`TAINT`, `ELIM`, `REGCLASS`, `ASM`). Each file is compiled through the full
pipeline and checked at each stage.
