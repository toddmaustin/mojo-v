# Mojo-V Data Contract Multitool (`dc-tool`) Documentation

## Overview

This document explains **what Mojo-V data contracts are**, **why they exist**, and **how to use the `dc-tool` multitool** to:

1. Generate ML-KEM-512 keypairs for Mojo-V CPU targeting.
2. Generate Mojo-V-compliant encrypted data contracts.
3. Validate encrypted data contracts.

It also clarifies an important boundary:

- `dc-tool` can validate/decrypt contracts **only when you provide a software private key file**.
- For a **real Mojo-V CPU**, the private key is intended to be hardware-bound and inaccessible, so end users cannot pass that key into `dc-tool` for decryption.

---

## 1) Mojo-V data contracts: why, what, and how

### Why

Mojo-V secure computation needs a compact, machine-readable payload that can securely carry:

- an ephemeral symmetric key,
- metadata/signature values,
- format selection for execution modes.

A data contract provides this payload and allows a data owner to transfer contract material so only the intended Mojo-V target can recover and use it.

### What

In this implementation, the data contract is a fixed **64-byte (512-bit)** structure with fields for:

- `salt` (64-bit random)
- `sig` (16-byte magic/version string: `"Mojo-V ver. #001"`)
- `sym_key_128` (128-bit random key)
- `contract_sig` (64-bit random value)
- `ciphers` (64-bit mask; currently `0`)
- `format_sel` (mode selector: fast/strong/proof-carrying)
- `pad` (7 bytes random padding)

The tool encodes this struct into a 64-byte wire format before encryption.

### How (high-level flow)

1. Data owner gets a Mojo-V target public key (`ML-KEM-512`).
2. `dc-tool dcgen` encapsulates to that public key, producing:
   - `KEM_DC` (KEM ciphertext)
   - `ss` (shared secret)
3. Tool derives a SIMON-128 key from `ss` via HKDF-SHA256.
4. Tool creates and encodes the 64-byte data contract.
5. Tool encrypts contract bytes with SIMON-128 (CBC-like chaining in code).
6. Tool writes an output text file containing:
   - `KEM=ML-KEM-512`
   - `CIPHER=SIMON-128`
   - `KEM_DC=<hex>`
   - `MSG_DC=<hex>`
   - `PT_SHA256=<hex>` (integrity fingerprint of plaintext contract)

---

## 2) Roles in Mojo-V secure computation

## Data owner

The **data owner** is the party preparing protected input/contracts for Mojo-V execution. In this tool’s model, the data owner:

- has access to the Mojo-V CPU’s public key,
- creates an encrypted contract (`dcgen`),
- sends that encrypted artifact toward execution/validation.

## Service provider

The **service provider** operates Mojo-V compute infrastructure (e.g., hosts CPUs/runtimes that execute secure workloads). In a production secure-hardware model:

- service infrastructure exposes or distributes public keys,
- private keys stay bound to hardware trust boundaries,
- contract decryption happens inside trusted Mojo-V execution context, not by arbitrary external users.

---

## 3) Public/private keys and ML-KEM-512 key encapsulation

`dc-tool` uses OpenSSL’s `ML-KEM-512` support through EVP APIs.

### Key concepts

- **Public key**: safe to share; used by the data owner to encapsulate a shared secret.
- **Private key**: secret; used by the receiving side to decapsulate and recover the same shared secret.

### Why this is useful

The data owner does **not** need to transmit the symmetric key directly. Instead:

1. Data owner encapsulates to the public key.
2. Encapsulation yields:
   - KEM ciphertext (`KEM_DC`) and
   - shared secret `ss`.
3. Recipient (with private key) decapsulates `KEM_DC` to recover matching `ss`.

This lets the owner and receiver derive the same encryption key material without exposing the receiver private key or directly sending plaintext key material.

### In this tool

- `ss` is fed into HKDF-SHA256 with fixed salt/info labels.
- HKDF output is a 128-bit key used by SIMON-128 to encrypt/decrypt the 64-byte contract payload.

---

## 4) Using the DC multitool to create Mojo-V CPU public/private keys

> From `dc-tool/` directory.

### Build

```bash
make build
```

### Generate keypair

```bash
./dc-tool keygen pk-file.pem sk-file.pem
```

Output:

- `pk-file.pem`: ML-KEM-512 public key (shareable)
- `sk-file.pem`: ML-KEM-512 private key (sensitive)

In a real deployment, private keys should be hardware-protected and not exported as regular files.

---

## 5) Using the DC multitool to create Mojo-V compliant data contracts

Generate a contract ciphertext file using a recipient public key:

```bash
./dc-tool dcgen pk-file.pem strong dc-file.txt
```

Mode options:

- `fast`  -> `format_sel = 0`
- `strong` -> `format_sel = 1`
- `proof-carrying` (or `proofcarrying`) -> `format_sel = 2`

The produced `dc-file.txt` includes all components needed for later validation by a holder of the matching private key.

### Example end-to-end (software simulation)

```bash
./dc-tool keygen pk-file.pem sk-file.pem
./dc-tool dcgen pk-file.pem proof-carrying dc-file.txt
./dc-tool dcchk sk-file.pem dc-file.txt
```

For verbose decrypted-field output (test/development):

```bash
./dc-tool dcchk-v sk-file.pem dc-file.txt
```

---

## 6) Validating data contracts with the DC multitool + hardware-key limitation

Validation command:

```bash
./dc-tool dcchk <sk_file> <ct_file>
```

What validation checks:

1. Parses `KEM_DC`, `MSG_DC`, and `PT_SHA256` from file.
2. Decapsulates `KEM_DC` with provided private key.
3. Derives SIMON key via HKDF.
4. Decrypts `MSG_DC`.
5. Recomputes plaintext SHA-256 and compares against `PT_SHA256`.
6. Verifies contract header signature bytes equal `"Mojo-V ver. #001"`.

### Why this cannot decrypt a secure contract for a real Mojo-V CPU

For real secure hardware, the private key is expected to be:

- device-bound,
- inaccessible/export-restricted,
- not provided to users as a PEM file.

`dc-tool dcchk` requires a user-supplied `<sk_file>`. If the private key is sealed inside Mojo-V hardware, users cannot provide that key to the tool, so they cannot perform external software decryption of production contracts.

In other words, `dcchk` is excellent for development/test validation with software keys, but not a bypass for hardware trust boundaries.

---

## Additional practical guidance

- Treat `sk-file.pem` as sensitive secret material.
- Rotate keys and regenerate contracts as part of operational hygiene.
- Use `dcchk-v` only in trusted debug environments because it prints decrypted fields.
- Keep contract files immutable after generation; tampering should fail validation via hash/header checks.
- Consider auditable key provenance for public keys distributed to data owners.

---

## CLI quick reference

```bash
./dc-tool keygen <pk_file> <sk_file>
./dc-tool dcgen <pk_file> {fast,strong,proof-carrying} <ct_file>
./dc-tool dcchk <sk_file> <ct_file>
./dc-tool dcchk-v <sk_file> <ct_file>
```

