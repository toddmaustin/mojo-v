<p align="center">
  <img src="./docs/MojoV-logo1.png" alt="Mojo-V Logo" width="550"/>
</p>

# Mojo-V: Secret Computation for RISC-V

**Mojo-V** (pronounced *“mojo-five”*) is a new RISC-V extension that introduces privacy-oriented programming capabilities for RISC-V. Mojo-V implements secret computation, enabling secure, efficient, and data-oblivious execution without reliance on fragile software and programmer trust. By sequestering sensitive data in dedicated secret registers and encrypting memory under a third-party key, Mojo-V prevents disclosures and enforces computation that is both blind (no direct disclosures) and silent (no side channel leakage). The design integrates seamlessly into the existing RISC-V ISA with only a mode bit and four new instructions, enforced entirely at decode. Early results show near-native execution speeds while offering over 5-7 orders of magnitude performance improvement compared to fully homomorphic encryption (FHE), with a clear roadmap for integration into CPUs, GPUs, and specialized accelerators.

To learn more...
- Here is an intro video describing Mojo-V: https://www.youtube.com/watch?v=HUT46TcNyyM
- Slides that give an overview of the Mojo-V project:  https://drive.google.com/file/d/1VVzZqYHvQgnKMgXZjg7I_cX2GzF7awSN

The current Mojo-V ISA Extension Specification (release 1.00):
- [In PDF format.] (https://drive.google.com/file/d/1O50EuI95i0APeQfc4jVVHrMJqdQnsEra)

To contact the developers of Mojo-V:
- Email: [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)

# 🧩 Mojo-V Reference Platform — Release 1.1

## 🚧 Project Status

The Mojo-V reference platform release 1.1 builds on the 1.00 full reference implementation in the Spike RISC-V simulator for an RV64GC CPU with ML-KEM-512 key encapsulation and SIMON-128 secret computation protection. Mojo-V continues to support fast, strong, and proof-carrying encryption modes, and this release expands Bringup-Bench workload coverage and EXO developer documentation.

As of release 1.1, 64-bit secret computation remains fully secretized for software development and security analysis, with continuing roadmap items including LLVM compiler support and broader ecosystem integration.

**Specification Version:** 1.00  (March 2026)  
**Contact:** [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)

## Current components

1. **Mojo-V ISA Spec v1.00**

   - released in `doc/`

2. **Spike (Instruction Set Simulator) with Mojo-V Extensions**

   - Mojo-V integrated into `riscv-isa-sim`, and feature-complete for an RV64GC CPU with ML-KEM-512 key encapsulation for data contract loading, and SIMON-128 symmetric key encryption for secret computation protection.
   - To run Spike with Mojo-V extensions enabled, add the `--isa=rv64gc_zicond_zkmojov_zicntr` flag when running `spike`

3. **Data Contract Multi-tool**

   Data contracts are encrypted packets that allow a Mojo-V CPU's hardware to access the data access key and configuration information (e.g., memory encryption mode) for a Mojo-V encrypted data set. The DC Multi-tool enables the following capabilities:

   * Hardware developers can create public/private ML-KEM512 key pairs: public keys are shared with service providers, private keys are embedded into the Mojo-V hardware implementation.
   * Data owners can create data contracts and encrypt them under the public ML-KEM512 keys of service providers. The matching Mojo-V hardware can then perform secret computation on the protected 3rd-party encrypted data.

4. **Mojo-V Bringup-Bench Benchmarks**

   - Hand-coded examples (e.g., bubble-sort) showing Mojo-V working secret computation
   - Full battery of security tests for RV64GC+Mojo-V
   - Full battery of integrity attack tests for RV64GC+Mojo-V

Note, the remainder of the Bringup-bench benchmarks have NOT been ported to Mojo-V, as yet.

## ⚙️ Building and Running the Mojo-V Reference Platform

### A. Install a RISC-V LLVM Compiler
You’ll need an LLVM-based RISC-V cross-compiler capable of producing `RV64GC` binaries.

Here is a good place to start: https://github.com/openssl/openssl

### B. Install OpenSSL version 3.6 or newer

You’ll need a developer's installation of OpenSSL version 3.6 or newer. This provides libraries that implement ML-KEM512, used by Spike for protected key exchange.

Here is a good place to start: https://clang.llvm.org/get_started.html

### C. Clone the Mojo-V Repository

```bash
git clone https://github.com/toddmaustin/mojo-v.git
cd mojo-v
```

### D. Build the RISC-V Spike simulator with Mojo-V Support
```bash
sudo apt-get install device-tree-compiler libboost-regex-dev libboost-system-dev
cd riscv-isa-sim
mkdir build
cd build
../configure --prefix=$RISCV
make
```

### **E. Build and test the Data Contract Multi-tool**

Data contracts are encrypted packets that allow a Mojo-V CPU's hardware to access the data access key and configuration information (e.g., memory encryption mode) for a Mojo-V encrypted data set.

```
cd dc-tool
make clean build test
```

### **E. Build and Run Mojo-V Bringup-Bench Benchmark Tests**

1. **Build the Spike device driver**

   ```bash
   cd bringup-bench/target
   make
   ```

2. **Configure your compiler**

   Edit `../Makefile` and set `TARGET_CC` for the `mojov` target to the location of your LVM Clang-based RISC-V compiler.

3. **Build and test the Bringup-Bench test programs**

   ```bash
   cd ..                # go to the top-level bringup-bench directory
   make mojov-tests     # run all Mojo-V tests
   ```

   As an alternative, you can run an individual benchmark by going into its directory and running the following command.

   ```bash
   cd ../mojov-test
   make TARGET=mojov clean build test
   ```



## 🧪 Mojo-V Bringup-Bench Tests Overview

| Program | Description |
|:---------|:-------------|
| `mojov-test` | Intro example from slides |
| `mojov-test1` | Secret-register and encrypted-memory semantics tests |
| `mojov-test2` | Hand-coded data-oblivious integer bubble-sort benchmark with Mojo-V fast encryption (int,fast) |
| `mojov-test3` | Hand-coded data-oblivious floating-point bubble-sort benchmark with Mojo-V fast encryption (fp,fast) |
| `mojov-test4` | Hand-coded data-oblivious integer bubble-sort benchmark with Mojo-V strong encryption (int,strong) |
| `mojov-test5` | Hand-coded data-oblivious floating-point bubble-sort benchmark with Mojo-V strong encryption (fp,strong) |
| `mojov-typetests` | Type-system validation tests for Mojo-V encrypted types and EXO-library usage |
| `mojov-pctests` | Hand-coded integrity checking test suite for RV64GC+Mojo-V that includes positive and negative tests for Mojo-V's proof-carrying encryption format (proofcarrying) |
| `mojov-sectests` | Hand-coded security test suite for RV64GC+Mojo-V that includes 130 pos + 245 neg tests == 375 total (int,fp,fast,strong) |

All test benchmarks are hand-coded assembly programs demonstrating Mojo-V ISA rules and security semantics.

## 🧪 Mojo-V Bringup-Bench Benchmarks Overview

The current bring-up benchmark set includes 40 Mojo-V benchmark applications:

| Program | Description |
|:---------|:-------------|
| `bitonic-sort` | Data-oblivious bitonic sorting benchmark |
| `bubble-sort` | Integer bubble-sort benchmark |
| `bubble-sort-strong` | Integer bubble-sort benchmark configured for strong encryption |
| `distinctness` | Distinctness analysis benchmark |
| `distinctness-Onlog2n` | Distinctness benchmark variant with O(n log² n) strategy |
| `edit-distance` | Edit-distance (string distance) benchmark |
| `eulers-approx` | Euler constant/series approximation benchmark |
| `fft-int` | Integer FFT benchmark |
| `flood-fill` | Flood-fill benchmark |
| `flood-fill-On2` | Flood-fill benchmark variant with O(n²) behavior |
| `gcd-list` | Greatest-common-divisor over list benchmark |
| `gemm` | General matrix multiplication benchmark |
| `gemm-strong` | GEMM benchmark configured for strong encryption |
| `grad-descent` | Gradient-descent optimization benchmark |
| `kadane` | Maximum-subarray benchmark (Kadane’s algorithm) |
| `kalman-filter` | Kalman filtering benchmark |
| `kcore-decomp` | Graph k-core decomposition benchmark |
| `kepler-calc` | Kepler equation/numerical calculation benchmark |
| `knapsack` | Knapsack optimization benchmark |
| `lda` | Latent Dirichlet allocation benchmark |
| `mersenne` | Mersenne-number computation benchmark |
| `minspan` | Minimum spanning structure benchmark |
| `nonlinear-nn` | Non-linear neural-network benchmark |
| `nr-solver` | Newton-Raphson solver benchmark |
| `ntt-kernel` | Number-theoretic transform kernel benchmark |
| `pagerank` | PageRank graph benchmark |
| `parrondo` | Parrondo process/strategy benchmark |
| `primal-test` | Primality testing benchmark |
| `private-join` | Privacy-preserving join benchmark |
| `psi` | Private-set-intersection benchmark |
| `rabinkarp-search` | Rabin-Karp pattern-search benchmark |
| `rad-to-deg` | Radian-to-degree conversion benchmark |
| `randshell-sort` | Randomized Shell-sort benchmark |
| `risk-score` | Risk-scoring analytics benchmark |
| `shortest-path` | Shortest-path graph benchmark |
| `skeleton` | Skeleton/template benchmark used as a bring-up baseline |
| `soundex` | Soundex phonetic encoding benchmark |
| `string-search` | String-search benchmark |
| `tea-cipher` | TEA cipher benchmark |
| `triangle-count` | Triangle counting graph benchmark |



## 🛠️ Mojo-V Data Contract Multi-tool Usage

The data contact multi-tool "dc-tool" is used to create and validate Mojo-V data contracts. 

To create an ML-KEM512 public/private key pair, execute the following command. Note that the public key is to be shared with 3rd-party data providers to prepare data contracts. Private keys are installed into the hardware (or simulator).

```bash
./dc-tool keygen <pk_file> <sk_file>          # public key in <pk_file>, private key in <sk_file>
```

Once a public/private key pair exists, it is then possible to create encrypted data contracts. A data contract contains an encrypted data access key (for Mojo-V hardware to access 3rd-party data) and an encrypted memory mode configuration. Execute the following command to created an encrypted data contract.

```bash
./dc-tool dcgen <pk_file> {fast,strong,proof-carrying} <ct_file>    # specify mem mode, contract in <ct_file>
```

Sharing an encrypted data contract with the Mojo-V hardware that corresponds to the public ML-KEM512 key used to encrypt the contract will allow the Mojo-V enabled CPU to perform secret computation on the protected 3rd-party data. To validate that the encrypted contract is valid, use the following commands.

```bash
./dc-tool dcchk <sk_file> <ct_file>           # decrypt contract <ct_file> with secret key <sk_file>
./dc-tool dcchk-v <sk_file> <ct_file>         # same as above, but also dump decrypted contents of <ct_file>
```



## 🛠️ Mojo-V Specific Options Added to RISC-V Spike ISA Simulator

The following options have been added to Spike, the standard RISC-V ISA simulator.

```bash
  --mojov-verbose       Mojo-V setup processing is verbose
  --mojov-fast          Use Mojo-V fast encryption mode (default mode)
  --mojov-strong        Use Mojo-V strong encryption format (otherwise using data contract specified mode)
  --mojov-proofcarrying Use Mojo-V proof-carrying encryption format (otherwise use data contract specified mode)
  --mojov-arg=<n>       Pass a numeric argument to a Mojo-V test code
  --mojov-pk=<pem_file> Load Mojo-V CPU public key from <pem_file>
  --mojov-sk=<pem_file> Load Mojo-V CPU secret key from <pem_file>
```

## 🧠 Mojo-V Programming Overview

Mojo-V software development currently uses the EXO compatibility library and follows secure data-oblivious coding practices:

1. **Program with the EXO library headers**
   - Include `exo/mojov-exo.h` to access the Mojo-V programming framework and encrypted-type abstractions.
   - Include `exo/mojov-math.h` to access encrypted math support and helper operations.

2. **Use data-oblivious computation for encrypted variables**
   - Encrypted values must be manipulated with data-oblivious control flow and memory-access patterns to preserve Mojo-V’s silent execution and side-channel resistance goals.

3. **Follow the EXO programming tutorial**
   - See the EXO programming guide: [exo/EXO-library-programming.md](./exo/EXO-library-programming.md)

4. **See compiler-structure and architecture details in EXO documentation**
   - Internal structure and design context for the current Mojo-V compiler/library approach are documented in: [exo/EXO-library-overview.md](./exo/EXO-library-overview.md)

---
## Code Licensing
All of the Mojo-V related code in this repo is released under the license of the tool it modified (e.g., Spike, LLVM, Bringup-Bench). Please see the tools' respective directories for licensing details.

---

## 💬 Questions & Feedback
We welcome contributions, bug reports, and suggestions!

📧 **Email:** [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)  
🌐 **Project Home:** [https://github.com/toddmaustin/mojo-v](https://github.com/toddmaustin/mojo-v)
