# DMEI: Dynamic Memory Encryption and Indirection
A Proof-of-Concept implementation of a moving-target dispatch layer for hardware-register-bound secrets.

## Overview
DMEI is a defense-in-depth dispatch layer that prevents address resolution and control-flow hijacking without simultaneously holding all hardware-resident secrets. [cite_start]By combining AES-encrypted indirection, dual-trigger reshuffling, and per-lookup function-body integrity, it renders memory snapshots and static analysis obsolete[cite: 7, 8, 27].

## Core Features
* [cite_start]**Two-Hop AES Indirection**: Secrets (K, T, E) remain exclusively in CPU registers[cite: 64, 71].
* [cite_start]**Moving-Target Reshuffle**: Periodically rotates all secrets and table offsets to invalidate memory snapshots[cite: 66, 328].
* [cite_start]**Integrity Binding**: Per-lookup function-body integrity using keyed MACs binds code content and canonical addresses[cite: 29].
* [cite_start]**Caller IP Validation**: Enforces that functions can only be invoked from authorized call sites[cite: 31, 297].
* [cite_start]**ROP Defense**: Encrypted return-address stack binding[cite: 33, 349].

## Prerequisites
* **Architecture**: x86-64.
* [cite_start]**Hardware Support**: AES-NI instructions required for constant-time performance[cite: 397].
* **Compiler**: Must support `wmmintrin.h` and `immintrin.h` (GCC/Clang).

## Quick Start
1. Compile with `-maes` and `-msse4`.
2. Run the included POC.
3. Observe the `[DEBUG]` logs showing epoch rotation and integrity verification.

## Warning
This is a research-grade Proof of Concept. It is not intended for production systems. [cite_start]It assumes a trusted Ring 0 entity for initial key generation and requires hardware-level register protection assumptions[cite: 55, 56].

## License
MIT (or your chosen license)
