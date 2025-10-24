# Mojo-V ISA Extension Specification

**Document version:** 0.90  
**Mojo-V specification version (mojov_ver):** #0x1  
**Release date:** October 24, 2025  
**Feedback:** [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)  
**Latest version:** [https://github.com/toddmaustin/mojo-v](https://github.com/toddmaustin/mojo-v)

---

## 1. Introduction

Mojo-V (pronounced *"mojo-five"*) extends the RISC-V RV64 ISA to support **secret computation** — computation that is blind to developers and silent in that its behavior does not reveal secret values. It provides cryptographic-strength privacy with minimal performance overhead and enables processors to execute operations on encrypted data without exposing values to any software, developer, or IT operator.

Mojo-V enforces privacy entirely in hardware, rendering all software and developers untrusted. Every software-visible operation is either data-oblivious or cryptographically sealed within the processor.

Developed at the University of Michigan and Agita Labs, Mojo-V is the fifth generation in a lineage of secure architectures combining strong cryptography with secure hardware to advance system security and privacy:

```
Morpheus (I) → Morpheus II (II) → Sequestered Encryption Unit (III) → TrustForge Platform (IV) → Mojo-V (V)
```

---

## 2. Caveats

Mojo-V is an open project that is in progress. As such, not everything is finished, either because we don’t have the resources yet or we are waiting for community input. Community feedback and contributions are welcome as the specification evolves.

### Current Caveats
- Mojo-V is currently specified only for extending **RV64** RISC-V. (Would an RV32 extension be desired?)
- CSR register addresses and **lde/sde** opcode assignments are not finalized. These will be determined with the RISC-V community.
- Hardware ciphers are not yet specified. The specification currently defines only the mechanism to probe supported ciphers and the data contract field where the data owner specifies which ciphers are required to protect data. The open-source project currently uses **ML-KEM (Kyber)** for asymmetric encryption and **Simon-128** for symmetric encryption.

---

## 3. Secret Computation

Mojo-V enables privacy-preserving computation by ensuring that even untrusted cloud or edge infrastructure can process encrypted data safely and efficiently.

Imagine a smart-home security camera performing face and object recognition in the cloud. Without Mojo-V, the cloud must first decrypt video frames, making them accessible to service provider software, developers, or IT staff. Privacy thus depends entirely on the provider’s internal trustworthiness.

With Mojo-V, the camera encrypts each video frame using the data owner’s key and transmits both the encrypted data and a third-party encrypted **data contract** to a Mojo-V-enabled cloud server. The Mojo-V processor decrypts and analyzes the data entirely within hardware, so plaintext video is never visible to software or operators. Even a compromised OS or malicious programmer cannot view, log, or leak contents. Trust shifts completely into hardware, removing dependence on software for data privacy.

### 3.1 Blind and Silent Computation

Mojo-V implements **blind computation** through secret registers and specialized semantics that strictly confine secrets within the processor. Data may enter secret registers only from third-party encrypted loads. Once marked secret, all derived results automatically inherit secrecy, requiring storage in secret registers. The only legal exit path is via a third-party encrypted store, which re-encrypts data using the data owner’s key and authenticated encryption.

**Silent computation** ensures no observable program behavior reveals secret information. Hardware rules enforce complete **data-obliviousness**:

1. **No secret registers in branch predicates** — prevents timing or control-flow leaks.
2. **No secret code pointers or return addresses** — prevents instruction fetch leakage.
3. **No secret memory addresses** — prevents cache, TLB, or page-fault leakage.

Even under these restrictions, any Mojo-V computation can be expressed using a data-oblivious model (conditional moves, oblivious lookup tables, fixed access patterns). Programmers write normal code, and the Mojo-V ISA and compiler enforce secrecy rules.

By combining blind and silent computation, Mojo-V guarantees privacy through silicon itself, not software discipline.

---

## 4. Mojo-V Security Model

A **security model** defines protection scope, trusted elements, and attacker capabilities. Mojo-V assumes **zero software trust**: all software (OS, hypervisors, admins) may be compromised. Only Mojo-V hardware and data-owner channels are trusted.

| Entity | Trust Level | Rationale |
|--------|--------------|------------|
| Software | Untrusted | Software compromise cannot reveal or alter protected data. |
| Programmers | Untrusted | Malicious application logic cannot intentionally or accidentally expose secrets. |
| IT / OS Staff | Untrusted | Administrative privileges provide no access to secret registers or encrypted memory. |
| Hardware (non-secret) | Untrusted | Non-Mojo-V hardware only handles ciphertext or public data. |
| Hardware (Mojo-V) | Trusted | Performs all secret computation on decrypted data inside sealed hardware registers. |

Attackers may control all software layers, execute arbitrary code, and attempt side-channel analysis. Yet their reach stops at the hardware boundary. They cannot read secret registers, extract plaintext via debug ports, or forge ciphertext without valid keys and signatures. Authenticated encryption ensures integrity and replay protection.

### Attack Success Criteria
1. **Secret disclosure** – obtaining plaintext or key material.
2. **Ciphertext forgery** – introducing falsified ciphertext that passes authentication.
3. **Replay compromise** – reusing old ciphertext to alter computation.

Mojo-V prevents all three through hardware-enforced confidentiality, integrity, and freshness.

---

## 5. Mojo-V ISA Specification

### 5.1 Configuration CSRs

| CSR Name | Address | Access | Purpose |
|-----------|----------|---------|----------|
| `mojov_cfg` | 0x0A0 | R/W | Controls secret configuration enable (`mojov_en`), key validity, encryption format, and zeroization state. |
| `mojov_ciphers` | 0x0A1 | RO | 64-bit mask of supported asymmetric and symmetric ciphers. |
| `mojov_pubkey` | 0x0A2 | RO | Processor public key for data-owner attestation. |
| `mojov_keycfg` | 0x0A3 | WO | Installs a 512-bit encrypted data contract defining the data owner’s key, permissions, and authentication hash. |
| `mojov_keystate` | 0x0A4 | R/W | Stores encrypted key state for context save/restore. |

**`mojov_cfg` Bitfields**

| Bits | Field | Access | Description |
|------|--------|---------|--------------|
| 0 | `mojov_en` | R/W | Enables secret computation mode. |
| 1 | `key_valid` | RO | Indicates a valid data contract is installed. |
| 2 | `format_sel` | RO | Encryption format selector (0 = weak, 1 = strong). |
| 10:3 | `mojov_ver` | RO | 8-bit version ID for Mojo-V implementation. |

### 5.2 Exceptions

A **Mojo-V Security Exception** (code 0x1F) is raised if a program attempts to:
- Move a secret value to a public register or memory.
- Use a secret value as a branch predicate, address, or code pointer.
- Perform a `secret→non-secret` conversion.
- Violate any secrecy rule from Section 3.

Exceptions are detected at **decode time**, before execution, ensuring no secret-dependent execution. Secret-mode instructions drop runtime exceptions (e.g., divide-by-zero) instead of propagating them, maintaining data-oblivious semantics.

### 5.3 Secret Registers

Registers `x28–x31` and `f28–f31` are **secret-capable**. When `mojov_en=1`, they become secret general-purpose (`p0–p3`) and secret floating-point (`pf0–pf3`) registers. When disabled, they revert and are immediately zeroized.

Rules:
- If any input is secret, the destination must be secret.
- Secret registers cannot serve as branch predicates, addresses, or code pointers.
- `fmv` and `fcvt` only permit `secret→secret` or `public→secret` transfers.
- All exceptions during secret operations are masked.

### 5.4 Third-Party Encrypted Loads and Stores

**LDE (Load Encrypted)** and **SDE (Store Encrypted)** handle ciphertext under a data contract.

#### Weak Format
```c
struct {
  uint64_t val;
  uint32_t salt;
  uint32_t auth_sig;
} pt_weak; // 128 bits
```

#### Strong Format
```c
struct {
  uint64_t val;
  uint64_t salt;
  uint64_t auth_sig;
  uint64_t metadata;
} pt_strong; // 256 bits
```

### SDE Instruction
```asm
sde rs2, offset(rs1)
```
Encrypts and stores a secret register value to memory using authenticated encryption.

### LDE Instruction
```asm
lde rd, offset(rs1)
```
Loads and decrypts ciphertext from memory into a secret register, verifying `auth_sig` for freshness.

Encryption must use a block cipher with strong **avalanche diffusion** to ensure that a single-bit change in plaintext or ciphertext affects all output bits.

### 5.5 Key Management

Keys are managed entirely in hardware. The **data contract** defines keys, formats, and authentication data:

```c
struct {
  uint8_t sig[16]; // "Mojo-V ver. #001"
  uint128_t sym_key_128;
  uint64_t auth_sig;
  uint64_t salt;
  uint64_t ciphers;
  bool format_sel;
  uint8_t __pad[]; // to 512 bits
} data_contract_t;
```

Installation occurs through `mojov_keycfg`, encrypted under `mojov_pubkey`. If validation passes, the `key_valid` bit is set.

---

## 6. Microarchitectural Considerations

- **Decode-time enforcement:** Violations trigger exceptions before execution.
- **Constant-latency units:** Execution time must not depend on secret data.
- **Speculation safety:** Prohibit secret→non-secret speculative forwarding.
- **Exception masking:** Drop secret-related faults; never expose timing side channels.

---

## 7. System Software Considerations

### Compiler
- LLVM-based toolchain under development.
- Supports `secret` variable qualifiers and `mojov_cmov()` intrinsic.
- Enforces compile-time errors for illegal secret use (e.g., branch predicates).

### Operating System
- Must preserve secret register state across context switches using `LDE`/`SDE`.
- Unauthorized saves (e.g., `sd`) cause Mojo-V security exceptions.

### Libraries
- **Compatibility builds:** Link non-secret libraries when compiling with `-fmojov`.
- **Data-oblivious variants:** Provide secure math, sorting, and string libraries.

### Debugging
- Debugging never exposes plaintext; only developer test contracts allow decrypted inspection.

---

## 8. Authorship

**Primary Author:** Todd Austin, University of Michigan / Agita Labs ([austin@umich.edu](mailto:austin@umich.edu))  
**Contributors:** Lauren Biernacki, Shibo Chen, Meron Demissie, Yonathan Fisseha, Tarunesh Verma

---

## 9. Change Log
- **v0.90 (Oct 24, 2025):** Initial release.

---

## Appendix A. Comparison to FHE

| Aspect | FHE | Mojo-V |
|--------|-----|---------|
| Foundation | Mathematical (lattice-based) | Architectural (hardware isolation) |
| Speed | 10 million× slower | Near-native hardware speed |
| Privacy basis | Cryptographic intractability | Hardware-enforced secrecy |
| Vulnerabilities | Physically secure | Susceptible to analog measurement |
| Use class | Military-grade | Consumer-grade |

Future versions will add zero-knowledge proof integration for hybrid verifiability.

---

## Appendix B. Comparison to CHERI

| Dimension | CHERI | Mojo-V |
|------------|--------|---------|
| Security Objective | Prevent software vulnerabilities | Protect secret data even under compromise |
| Mechanism | Capability-based memory safety | Cryptographically enforced secrecy domain |
| Software Trust | Requires trusted compiler & OS | Zero software trust |
| Protection Granularity | Per-pointer/object bounds | Per-register/block encryption |
| Failure Model | Prevents corruption | Tolerates compromise (yields ciphertext) |
| Overhead | Low–moderate | Near-zero |
| Extension | Capability registers, tagged memory | Secret registers, encryption CSRs |

Mojo-V complements CHERI by protecting data rather than code integrity. CHERI hardens *software correctness*; Mojo-V hardens *data confidentiality*.

---


