# Mojo-V EXO Library Programming Guide

This guide focuses on **how to write programs with EXO encrypted types** in C++: how to define encrypted variables, which operations are supported, what data-oblivious restrictions to follow, and concrete coding patterns.

## 1) Getting started

Include the EXO header in C++ code:

```cpp
#include "exo/mojov-exo.h"
```

EXO provides encrypted scalar templates and aliases:

- Integers: `exo::inte_t<Bits, IsSigned>`
- Floating point: `exo::fpe_t<Bits>`
- Common aliases:
  - `uint8e_t`, `uint16e_t`, `uint32e_t`, `uint64e_t`
  - `int8e_t`, `int16e_t`, `int32e_t`, `int64e_t`
  - `fp32e_t`, `fp64e_t`

You can use either fully-qualified names (`exo::uint64e_t`) or global `using` aliases exposed by the header.

---

## 2) Defining encrypted variables

### A) Integer encrypted variables

```cpp
uint64e_t a = 10;            // encrypted unsigned 64-bit
int32e_t  b = -7;            // encrypted signed 32-bit
exo::inte_t<13, false> c = 5; // encrypted unsigned 13-bit
exo::inte_t<9, true>   d = -3; // encrypted signed 9-bit
```

### B) Floating-point encrypted variables

```cpp
fp64e_t x = 3.14159;
fp32e_t y = 2.5f;
exo::fpe_t<64> z = 1.0;
```

### C) Construction and assignment notes

- Constructing from plaintext automatically encrypts the provided value.
- Assignment from plaintext (`a = 42`) is supported.
- Assignment from encrypted storage payloads is also supported through the EXO wrapper APIs.
- In practice, **encrypted operations can mix encrypted and plaintext inputs** (for example `enc_x + 7`, `enc_y * 3.5`, `enc_a < 10`).

### D) Optional storage ABI customization

Before including `mojov-exo.h`, you can select backing encrypted-memory payload formats:

```cpp
#define EXO_UINT64E_STORAGE_TYPE mojov_mem_strong_u64_t
#define EXO_FP64E_STORAGE_TYPE   mojov_mem_strong_fp64_t
#include "exo/mojov-exo.h"
```

That allows the same source program to target fast/strong/proof-carrying storage layouts.

---

## 3) Operations on encrypted variables

EXO overloads operators so encrypted code looks close to standard C++.

### A) Integer arithmetic/bitwise

Supported encrypted integer forms include:

- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Bitwise: `&`, `|`, `^`, `~`
- Shifts: `<<`, `>>`
- Unary/sign-style: unary `+`, unary `-`, logical-not `!`
- Compound assignment: `+=`, `-=`, `*=`, `/=`, etc.
- Increment/decrement: `++`, `--`

For these integer operators, inputs may be:

- encrypted + encrypted
- encrypted + plaintext integral literal/variable
- plaintext integral + encrypted (for supported binary forms)

Example:

```cpp
uint64e_t a = 10;
uint64e_t b = 3;
uint64e_t c = (a * b) + 7;
c ^= 0x55;
uint64e_t d = 100 - a;   // plaintext + encrypted input mix
```

### B) Floating-point arithmetic

Supported encrypted FP forms include:

- `+`, `-`, `*`, `/`
- unary `+`, unary `-`
- compound assignment (`+=`, `-=`, `*=`, `/=`)

FP operators also support mixed encrypted/plain inputs (for example `enc_fp + 1.25`, `2.0 * enc_fp`).

Example:

```cpp
fp64e_t x = 1.5;
fp64e_t y = 2.0;
fp64e_t z = (x * y) + 4.25;
fp64e_t w = 10.0 - x;    // plaintext + encrypted input mix
```

### C) Comparisons and predicates

Relational/equality operations on encrypted values return an **encrypted predicate** (not a plaintext `bool`). Comparisons also support mixed encrypted/plain inputs.

```cpp
uint64e_t a = 8;
uint64e_t b = 13;
uint64e_t pred = (a < b);    // encrypted predicate value
uint64e_t pred2 = (a < 20);  // encrypted-vs-plaintext input mix
```

### D) Boolean-style operators are non-short-circuit

EXO defines logical-style operators over encrypted integers without C++ short-circuit behavior, so both sides are evaluated.

```cpp
uint64e_t p = (a < b);
uint64e_t q = (b != 0);
uint64e_t both = (p && q);   // encrypted logical-and
uint64e_t any  = (p || q);   // encrypted logical-or
```

### E) Encrypted conditional select with `cmov`

Use `cmov` for secret-dependent selection:

```cpp
uint64e_t minv = cmov(a < b, a, b);
fp64e_t best = cmov(a < b, 1.25, 2.75);
uint64e_t s = cmov(a < b, a, 42);  // encrypted/plain branch value mix
```

`cmov` is overloaded for integer and floating-point encrypted types and mixed encrypted/plain operands.

---

## 4) Data-oblivious programming restrictions

When writing EXO code, treat encrypted data as secret and preserve a data-oblivious execution shape.

### Restriction 1: do not branch on encrypted predicates with `if/else`

Encrypted predicates must be handled with data-oblivious selection instead of control-flow branches.

Use this legal pattern:

```cpp
out = cmov(a < b, a, b);
```

`cmov` also accepts plaintext branch values when useful (`cmov(a < b, a, 0)`).

### Restriction 2: do not use encrypted values as loop-control conditions

Avoid making loop trip counts secret-dependent. Prefer fixed/public loop bounds and use masked/cmov-based updates in the loop body.

### Restriction 3: do not rely on short-circuit semantics for secret logic

In EXO, logical encrypted operators are explicit operations and evaluate both sides; write expressions assuming that behavior.

### Restriction 4: keep memory access patterns independent of secrets

Prefer regular scan/update patterns and predicate-based writes (`cmov`) rather than secret-indexed access that can leak information through access patterns.

### Restriction 5: only decrypt for debug/test workflows

Use `debug_context(...)` + `.decrypt()` only in controlled testing/validation paths. Normal secure program logic should stay in encrypted-domain operations.

---

## 5) Coding examples

## Example A: encrypted min/max and absolute difference

```cpp
#include "exo/mojov-exo.h"

void minmax_absdiff(uint64e_t a, uint64e_t b,
                    uint64e_t &mn, uint64e_t &mx, uint64e_t &absdiff) {
  uint64e_t pred = (a < b);
  mn = cmov(pred, a, b);
  mx = cmov(pred, b, a);
  absdiff = mx - mn;
}
```

## Example B: data-oblivious conditional accumulation

```cpp
#include "exo/mojov-exo.h"

uint64e_t score_masked(const uint64e_t *vals, const uint64e_t *flags, size_t n) {
  uint64e_t acc = 0;
  for (size_t i = 0; i < n; ++i) {
    // add vals[i] only when flags[i] is nonzero, without control-flow branch
    acc += cmov(flags[i] != 0, vals[i], 0); // plaintext false-branch input
  }
  return acc;
}
```

## Example C: branchless clamping for encrypted floating point

```cpp
#include "exo/mojov-exo.h"

fp64e_t clamp_fp(fp64e_t x, fp64e_t lo, fp64e_t hi) {
  fp64e_t t = cmov(x < lo, lo, x);   // t = max(x, lo)
  return cmov(t > hi, hi, t);        // min(t, hi)
}
```

## Example D: debug-time decryption checks (test harness)

```cpp
#include "exo/mojov-exo.h"

int verify_example() {
  if (debug_context(SIMON128_KEY, CONTRACT_SIG) != 0) return -1;

  uint64e_t a = 9;
  uint64e_t b = 4;
  uint64e_t m = cmov(a < b, a, b);

  uint64_t got = m.decrypt();   // debug/test only
  return (got == 4) ? 0 : 1;
}
```

---

## 6) Practical checklist for EXO programming

- Prefer encrypted types (`inte_t`/`fpe_t` aliases) end-to-end for secret values.
- Use operator overloads for arithmetic/logic; use `cmov` for secret-dependent selection.
- Remember that encrypted operations may take plaintext operands too, so you can write mixed expressions directly.
- Keep branches, loop bounds, and memory access patterns independent of encrypted secrets.
- Reserve `decrypt()` for validation/debug, after `debug_context(...)` initialization.
- If needed, set `EXO_*_STORAGE_TYPE` macros to pick your encrypted storage ABI.

This pattern keeps your code aligned with EXO’s encrypted execution and data-oblivious design intent.
