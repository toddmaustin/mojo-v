<p align="center">
  <img src="./docs/MojoV-logo.jpg" alt="Mojo-V Logo" width="450"/>
</p>

# Mojo-V: Secret Computation for RISC-V

**Mojo-V** (pronounced *“mojo-five”*) is a new RISC-V extension that introduces privacy-oriented programming capabilities for RISC-V. Mojo-V implements secret computation, enabling secure, efficient, and data-oblivious execution without reliance on fragile software and programmer trust. By sequestering sensitive data in dedicated secret registers and encrypting memory under a third-party key, Mojo-V prevents disclosures and enforces computation that is both blind (no direct disclosures) and silent (no side channel leakage). The design integrates seamlessly into the existing RISC-V ISA with only a mode bit and two new instructions, enforced entirely at decode. Early results show near-native execution speeds while offering over 5-7 orders of magnitude performance improvement compared to fully homomorphic encryption (FHE), with a clear roadmap for integration into CPUs, GPUs, and specialized accelerators.

To learn more...
- Here is an intro video describing Mojo-V: https://www.youtube.com/watch?v=HUT46TcNyyM
- Slides that give an overview of the Mojo-V project:  https://drive.google.com/file/d/1VVzZqYHvQgnKMgXZjg7I_cX2GzF7awSN

The current Mojo-V ISA Extension Specification (release 0.90):
- [In PDF format.] (https://drive.google.com/file/d/1IlRrDrWvsOj-reC-BC01QHwn6NAkF7bf)

To contact the developers of Mojo-V:
- Email: [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)

# 🧩 Mojo-V Reference Platform — Release 0.90

## 🚧 Project Status

The Mojo-V reference platform release 0.90 implements secret integer computation using a fixed symmetric key cipher. As of this release, 64-bit integer secret computation is fully secretized and this early reference platform can be used for software development and red-teaming. Additional capabilities will be rolled out in future releases, including PKI support, secret FP register support, LLVM compiler support, 32-bit RISC-V support, VIP-Bench benchmarks support, etc.

**Specification Version:** 0.90  (October 2025)  
**Contact:** [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)

## Current components

1. **Mojo-V ISA Spec v0.90** — released in `doc/`.
2. **Spike (Instruction Set Simulator) Implementation**  
   – Mojo-V integrated into `riscv-isa-sim`, nearly feature-complete.  
   – Missing only:  
     i) Secret floating-point (FP) support                       
     ii) strong encyption packet marshalling, currently only weak encryption format supported
     iii) Public-Key Infrastructure (PKI) support (currently uses fixed keys with a Simon-128 cipher)
   - to run Spike with Mojo-V extensions enabled, add the `--isa=rv64gc_zicond_zkmojov_zicntr` flag when running `spike`
3. **Mojo-V Bringup-Bench Benchmarks** — hand-coded examples showing Mojo-V semantics:  
   - `mojov-test`  – the simple Mojo-V code example from Mojo-V intro slides  
   - `mojov-test1` – a battery of secret-register + third-party encrypted-memory tests
   - `mojov-test2` – a hand-coded Mojo-V self-checking bubble-sort benchmark  

Note, the remainder of the Bringup-bench benchmarks have NOT been ported to Mojo-V, as yet.

## ⚙️ Building and Running the Mojo-V Reference Platform

### A. Install a RISC-V LLVM Compiler
You’ll need an LLVM-based RISC-V cross-compiler capable of producing `RV64GC` binaries.

Here is a good place to start: https://clang.llvm.org/get_started.html

### B. Clone the Mojo-V Repository
```bash
git clone https://github.com/toddmaustin/mojo-v.git
cd mojo-v
```

### C. Build the RISC-V Spike simulator with Mojo-V Support
```bash
sudo apt-get install device-tree-compiler libboost-regex-dev libboost-system-dev
cd riscv-isa-sim
mkdir build
cd build
../configure --prefix=$RISCV
make
```

### D. Build and Run Mojo-V Bringup-Bench Benchmark Tests

1. **Build the Spike device driver**

   ```bash
   cd bringup-bench/target
   make
   ```

2. **Configure your compiler**

   Edit `../Makefile` and set `TARGET_CC` for the `mojov` target to the location of your LVM Clang-based RISC-V compiler.

3. **Build and test**

   ```bash
   cd ../mojov-test
   make TARGET=mojov clean build test
   ```

   Repeat for:
   ```bash
   mojov-test1   # secret-register and encrypted-memory semantics tests
   mojov-test2   # data-oblivious bubble-sort in Mojo-V
   ```

## 🧪 Mojo-V Bringup-Bench Benchmarks Overview

| Program | Description |
|:---------|:-------------|
| `mojov-test` | Intro example from slides |
| `mojov-test1` | Secret-register and encrypted-memory semantics tests |
| `mojov-test2` | Hand-coded data-oblivious bubble-sort benchmark in Mojo-V |

All three test benchmarks are hand-coded assembly programs demonstrating Mojo-V ISA rules and security semantics. The other Bringup-Bench benchmarks have not yet been ported to Mojo-V.

---

## 💬 Questions & Feedback
We welcome contributions, bug reports, and suggestions!

📧 **Email:** [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)  
🌐 **Project Home:** [https://github.com/toddmaustin/mojo-v](https://github.com/toddmaustin/mojo-v)

