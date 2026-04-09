# VIP-Bench Benchmark Suite


## Introduction

This is the VIP-Bench Benchmark Suite, which is a collection of benchmarks to serve the evaluation of privacy-enhanced computation frameworks.
A privacy-enhance computation framework is one in which encrypted data can be directly processed without software needing to decrypt it.
These systems naturally enhance privacy since hacking into a system using privacy-enhanced computation only gives the attacker access
to ciphertext. Examples of privacy-enhanced computation frameworks that this benchmark suite wants to server are: homomorphic encryption,
runtime encryption, multi-party computation, etc.

*Why do we need a privacy-oriented benchmark suite?* Privacy-oriented programming has two primary impacts on software algorithms: first,
the algorithms become less heuristic since they can no longer inspect the data they are operating on; and second, the core datatypes will
be replaced with encrypted (and otherwise enhanced) data types. The VIP Benchmark Suite has already done this work for you, for a wide
range of application that suggest a strong need for data privacy.

*My privacy-enhanced computation framework doesn't work with some/all of the VIP benchmarks!* Yes, this may be the case
If you cannot support a benchmark, due to the data types it requires or the complexity of its
computation, perhaps you can benchmark without that particular application. Since some of the privacy-enhanced frameworks can do all of
these benchmarks, we felt it was important to take a greatest-common-demoninator approach to selecting benchmarks, rather than a
least-common-demoninator approach.  In any event, please help us build in support for your framework in our benchmark suite.

To learn more about VIP-Bench's design and implementation, please read the [VIP-Bench paper](https://drive.google.com/file/d/1aresSfrY_8C0gMtrcF0LRfTRGybESsA2/view?usp=sharing).

If you use the VIP-Bench benchmarks in your research and/or publications, please cite the VIP-Bench paper:

> Lauren Biernacki, Meron Zerihun Demissie, Kidus Birkayehu Workneh, Galane Basha Namomsa, Plato Gebremedhin, Fitsum Assamnew Andargie, Brandon Reagen, and Todd Austin, VIP-Bench: A Benchmark Suite for Evaluating Privacy-Enhanced Computation Frameworks, in the 2021 IEEE International Symposium on Secure and Private Execution Environment Design (SEED-2021), September 2021.

## VIP Benchmarks

The VIP benchmarks were selected for their (mostly) obvious demands for data privacy. Currently, the benchmark suite supports the following
benchmarks. Note that the benchmarks tagged with (HE-friendly) can be run on most homomorphic encryption frameworks because they have only linear operators and shallow computations. The remaining benchmarks are classified as HE-hard, since they would likely require deep computations and non-linear operator support.

- **Bitonic Sort** - Sorts an array of integers using the bitonic sort algorithm. This algorithm has O(Nlog^2n) complexity, compared to bubble sort's O(N^2) complexity. The integer values being sorted are completely privatized.

- **Boyer-Moore Search** - Implements the Boyer-Moore heuristic search algorithm. All string data, including the search database and the search string are privatized.

- **Bubble Sort** - Sorts an array of integers using the bubble sort algorithm. The integer values being sorted are completely privatized.

- **Distinctness** - Tests an array of integers to see if they are all disinct, returns the first duplicate. The test array values and their distinctness results are all privatized.

- **Distinctness O(Nlog^2N)** - Tests an array of integers to see if they are all disinct, returns the first duplicate. This algorithm works in O(nlog2n + n) time using bitonic sort, compared to the baseline distictness algorithm, which works in O(n^3) time. The test array values and their distinctness results are all privatized.

- **Edit Distance** - Computes the edit distance between short genetic sequences. The genomic input data and computed edit distances are all privatized.

- **Eulers Number Approximation** - Calculate e=2.718, using an interative approximation. The approximate value of e is privatized.

- **FFT Integer** - Computes an integer Fast Fourier Transform computation. The input signal values and the produced FFT analysis are all privatized.

- **Filtered Query** - A KissDB application that supports privatized home sales information, with a filtered query capability that returns a variety of statistics about home sales in the database. The historical home sales data and the queries statistical results are all privatized.

- **Flood Fill** - Performs a graphical flood fill operation on a 2D privatized image, with an algorithm that is naively O(N^4). The image being filled and the location of the start of the flood fill are all privatized.

- **Flood Fill O(N^2)** - Performs a graphical flood fill operation on a 2D privatized image, but this version is more clever and achieves O(N^2) data-oblivious performance, compared the original flood fill algorithms, which is O(N^4). The image being filled and the location of the start of the flood fill are all privatized.

- **GCD List** - Computes the greatest common divisor for a list of integers using the division algorithm. The input list of integers and the computed GCD for the list are all privatized.

- **Gradient Descent** - Computes the Gradient Descent of a fixed data set. The input integer data set and the computed gradient are all privatized.

- **Gradient of X** - Computes the gradient of vector X (HE-friendly). The vector X and the computed gradient are all privatized.

- **Image Filter** - Applies a sepia-tone filter to an image. The input image and the resulting filtered image are all privatized.

- **Kadanes Algorithm** - Finds the maximum sum of a contiguous subarray in a given privatized integer array. The input array of integers and the location of the maximum subsum are all privatized.

- **Kalman Filter** - Computes a smoothed Kalman filtering of accelerometer data from the 3-axis intertial measurement unit. The accelerometer data and all filter computation and results are privatized.

- **Keplers Equation Calculator** - Performs orbital trajectory calculations using five Kepler's Equation solvers. The orbital parameters and the computed trajectory are all privatized.

- **Knapsack** - Computes the 0-1 Knapsack solution for a set of weights and values. The input weights and values and the computed packing are all privatized.

- **Linear Discriminant Analysis** - Performs linear discriminant anaylsis of a privatized fixed data set. The input integer data set and the computed discriminant are all privatized.

- **Linear Regression** - Performs linear regression on a fixed privatized vector data set (HE-friendly). The input integer vector and the computed regression are all privatized.

- **Mersenne Twister** - A privatized random number generator based on Mersenne's Twister. The state of the RNG and the random values it produces are all privatized.

- **Minimum Spanning Tree** - Finds the minimal spanning tree of a graph (via Kruscal's algorithm over the graph's adjacency matrix). The graph and the discovered minimal spanning tree are privatized.

- **NMIST CNN** - A 7-layer CNN model performing training and inference on the MNIST handwriting data set. The CNN weights, inputs, CNN outputs are all privatized.

- **Nonlinear NN** - A collection of popular non-linear activation functions utilized in neural networks (e.g., LeakyReLU). The inputs to the activation functions and their computed results are all privatized.

- **Netwon-Raphson Solver** - Computes a square-root value using a Newton-Raphson solver. The inputs and outputs of the solver are privatized.

- **Pagerank** - Computes the page rank algorithm on a randomly generated graph. The input graph and the computed page rank are all privatized.

- **Parrondo's Paradox** - A game theory based solver that simulates Parrondo's paradox. The inputs and outputs of the solver are privatized.

- **Miller-Rabin Primality Test** - Performs the Miller-Rabin stochastic primarility test to extremely high certainty. The value being tested for primality and its outcome are all privatized.

- **Rabin-Karp Rolling Hash String Search** - Implements the very efficient Rabin-Karp data-oblivious string search algorithm. This search algorithm is O(N) in the length of the string searched. All string data, including the string search database and the search string are privatized.

- **Radians to Degrees** - Converts a vector of radian measurements to degrees. The input vector of radians and the computed degree values are all privatized.

- **Randomized Shell Sort** - Sorts an array of integers using Goodrich's data-oblivious randomized shell sort algorithm. This algorithm very efficient, having a complexity of O(NlogN) with the number of elements sorted. This The integer values being sorted are completely privatized.

- **Recommender** - Performs movie recommendations using the matrix factorization algorithm. The movies viewed by users and their ratings and the predicted ratings computed are all privatized.

- **Set Intersect** - Performs private set intersection (PSI) on two randomly generated sets of integers. The input sets and the computed intersection are all privatized.
 
- **Shortest Path** - Solves the all-pairs shortest path problem using the Floyd-Warshall algorithm. The input graph and the computed path are all privatized.

- **Soundex String Compare** - Compares two strings using the sounds-alike Soundex algorithm configured for the English language, such that the two strings compare as equivalent if they sound alike when spoken. This algorithm privatizes the strings being compared as well as the result of the comparison.

- **String Search** - Implements the moderately efficient naive data-oblivious string search algorithm. This search algorithm is O(N*M), where N is the the length of the string searched and M is the length of the string to be located. All string data, including the string search database and the search string are privatized.

- **TEA Cipher** - A TEA (Tiny Encryption Algorithm) cipher that runs on privatized data. The cipher key and inputs and the encrypted output are all privatized.

- **Triangle Count** - Computes the triangle count for a fixed privatized graph. The input graph and the computed triangle count are all privatized.


## Prerequisites

The "numdiff" utility must be installed, as it is used to compare results that may be slightly imprecise (e.g., kepler-calc in DO mode). This tool lets the test-runs verify that the answer is within a specific tolerance of the expected value.

## Building SEAL-3.4 Library
To use VIP-Bench with SEAL-3.4, you must first install and build the SEAL submodule repository:
```
git submodule update --init --recursive
cd lib/SEAL-3.4/SEAL/native/src
cmake .
make

```
Then you can continue building and runnign the VIP benchmarks from the parent directory. 


## Building and Running the VIP Benchmarks

First, you have to configure the benchmark suite, which requires creating two symbolic links in the top-level directory: one for the Makefile configuration and one for the C++ compilation configuration. If you have your own PEC framework, you will have to create the associated configuration files. We've included configuration files for the VIP Functional Library, which is a reference implementation of the VIP Computational Model using Intel's AES-NI instructions. You can configure the VIP-Bench to utilize this library with the following top-level Makefile commands:
```
make config-vip   # configure VIP-Bench to target the VIP-Bench functional library
make config-meso  # configure VIP-Bench to target Agita Labs' Mesosphere secure computation SDK
make config-seal  # configure VIP-Bench to target Microsoft's SEAL Homomorphic Encryption library
```
Or, it is possible to configure the libraries manually using the following commands:
```
ln -sf configs/config.mk.vip config.mk
ln -sf configs/config.h.vip config.h
```

Then, one can build, run and test the benchmarks in each of their respective directories. All benchmarks can be build in three distinct execution modes:

- **Native mode - MODE=na** - This mode runs the benchmark in its original non-privatized form. This *should* be the baseline that you compare against.

- **Data-oblivious mode - MODE=do** - This mode runs the benchmark in data-oblivious execution mode. In this mode, the benchmarks doesn't utilize any
encryption support, but it is also disciplined to never make decisions or access memory based on a privatized value. You can use this mode to better
understand the overheads associated with data-oblivious software changes versus changes needed to add encryption support to the privacy-enhanced
execution.

- **Encrypted mode - MODE=enc** - This mode runs the bencharks in its privatized mode, with data-oblivious execution and full encryption support.

Each benchmark support three standard Makefile targets: build, test, and clean

- **build** - Builds the benchmark

- **test** - Runs the benchmark and validates its output.

- **clean** - Deleted all derived files.

For example, to build, test and then clean the Bubble Sort benchmark in encrypted mode:
```
make MODE=enc build
make MODE=enc test
make MODE=enc clean
```

To assist in running experiments, the top-level Makefile includes a few useful targets:
```
make run-tests   # clean, build, and test all benchmarks in all target modes (NA,DO,ENC)
make all-clean   # clean all benchmark directories
```
You should be able to adapt these targets to your own project-specific tasks.

## Running VIP Benchmark Security Analysis

To assess the security of a privacy-enhanced computation framework, VIP-Bench performs indistinguishability analysis on sampled ciphertext from a running program. The ciphertext is analyzed using the DIEHARDER random test suite, which looks for any traces of information in the sampled ciphertext. To run these experiments, first install the DIEHARDER test suite (e.g., for Ubuntu):
```
sudo apt-get install -y dieharder
```

Next, build the SLICE-N-DICE application, which notably requires the underlying privacy-enhanced computation framework to implement the VIP_EMITCT() macro. To build and run the SLICE-N-DICE sampled ciphertext generator and then analyze the emitted ciphertext with DIEHARDER, use the following commands:
```
cd slice-n-dice
make MODE=enc clean build
./slice-n-dice SAMPLES.log 500000000
dieharder -a -g 202 -f SAMPLES.log
```

Note that the format of the SLICE-N-DICE command is: slice-n-dice \<sample-file-name\> \<total-samples\>. Also note that DIEHARDER will likely need about 500M to 1G of samples before its tests function properly without wrapping the input sample file. The DIEHARDER tests will likely take about 4-6 hours to run on a high-end system. When running DIEHARD, each of the tests will receive a "PASS", "FAIL" or "WEAK" score. Ideally, one wants to receive all "PASS" scores, but it is often the case even for high-quality true random sources that the occasional one or two "WEAK" scores will appear. Any occurance of a "FAIL" indicates either that 1) too few samples of ciphertext were made available (DIEHARDER will indicate how many times it rewound the input file), or 2) there is information (either based on secret data or cryptographic metadata) contained in your generated ciphertext. If it is the latter case, this situation will demand a deeper inspection of the ciphertext generation process.

## Porting the VIP Benchmarks to Another Privacy-Enhanced Computation Framework

The benchmarks suite is designed to be readily ported to other privacy-emhanced computation frameworks. To accomplish this, one needs to define
the approach "config.mk" and "config.h" in the directory "configs/". "config.mk" teaches the top-level makefile how to build the three modes
of benchmarks execution, including their target-specific includes and libraries. "config.h" teaches the VIP benchmarks how to access
privacy-enhanced programming features. Currently, these features include defining various privacy-enhanced data types, decrypting privacy-enhanced
data type (for validating test results only), and conditional move primitives. The underlying privacy-enhanced computation system is assumed to
support both fixed and floating point math.


## Porting New Benchmarks into the VIP Benchmark Suite

We are eager to include new benchmarks into the VIP Benchmark Suite, however, we ask contributors to limit their contributions to benchmarks that
accomplish a non-trivial goal that clear fills a privacy need. New benchmarks do not need to be large and complicated, though, but try to make
them more purposeful than the commonly found "multiply" or "max" benchmarks.

To port a new benchmark to the VIP Benchmark Suite, you need to adapt the application to the VIP_ interfaces defined in the "config.h" file.
These interfaces include:

- **VIP_*_MODE** - These #define's let the benchmark know which of the build modes are requested: native (VIP_NA_MODE), data-oblivious (VIP_DO_MODE),
and encrypted (VIP_ENC_MODE). Note that when VIP_ENC_MODE is defined, VIP_DO_MODE will also be defined, since encrypted variables will force
data-oblivious programming. We kindly ask that the native mode (VIP_NATIVE_MODE) represent the original version of the program, to the extent possible.

- **VIP_CHAR, VIP_INT...** - These #define's hook to the way to define encrypted data types. Note that when running in VIP_NA_MODE, these #define's
will default to the native types, e.g., "char", "int". Benchmarks can assume that the underlying system supports all C/C++ operators on encrypted
data, plus the availability of a LIBM-compatible math library.

- **VIP_CMOV** - To support limited control on encrypted values, the benchmarks assume the existance of a conditional move predicate with the following
semantics: VIP_CMOV(P,T,F) == P ? T : F. This predication primitive is used by the benchmarks to make decisions on encrypted data in a
data-oblivious manner.


## Licensing Details

The portions of the benchmark suite that was build by the VIP Benchmark team are (C) 2019-2021 and available for use under
the [Apache License, version 2.0](https://www.apache.org/licenses/LICENSE-2.0) 

## VIP Benchmark Suite Authors

- Fitsum Andargiae, Addis Ababa Institute of Technology
- Todd Austin, University of Michigan and Agita Labs
- Galane Basha, Addis Ababa Institute of Technology
- Lauren Biernacki, University of Michigan
- Shibo Chen, University of Michigan
- Meron Demissie, University of Michigan and Addis Ababa Institute of Technology
- Plato G Medhin, Addis Ababa Institute of Technology
- Brandon Reagen, New York University
- Biniyam Tiruye, University of Michigan
- Blandine Umuhoza, University of Rwanda
- Kidus Workneh, Addis Ababa Institute of Technology 

And, thanks to the respective authors of the benchmarks that were adapted for the VIP Benchmark Suite from other efforts.

