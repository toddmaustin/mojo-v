# Mojo-V: Secret Computation for RISC-V

Mojo-V is a new RISC-V extension that introduces secret computation, enabling secure, efficient, and data-oblivious execution without reliance on fragile software and programmer trust. By sequestering sensitive data in dedicated secret registers and encrypting memory under a third-party key, Mojo-V prevents disclosures and enforces computation that is both blind (no direct disclosures) and silent (no side channel leakage). The design integrates seamlessly into the existing RISC-V ISA with only a mode bit and two new instructions, enforced entirely at decode. Early results show near-native execution speeds while offering over 5-7 orders of magnitude performance improvement compared to fully homomorphic encryption (FHE), with a clear roadmap for integration into CPUs, GPUs, and specialized accelerators.

To learn more...
- Here is an intro video describing Mojo-V: https://www.youtube.com/watch?v=HUT46TcNyyM
- Slides that give an overview of the Mojo-V project:  https://drive.google.com/file/d/1VVzZqYHvQgnKMgXZjg7I_cX2GzF7awSN

The current Mojo-V ISA Extension Specification:
- [In markdown format.] (https://github.com/toddmaustin/mojo-v/MojoV-spec-0.90.md)
- [In PDF format.] (https://drive.google.com/file/d/1IlRrDrWvsOj-reC-BC01QHwn6NAkF7bf)

To contact the developers of Mojo-V:
- Email: [mojov-devs@umich.edu](mailto:mojov-devs@umich.edu)
