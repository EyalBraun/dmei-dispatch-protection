# DMEI: Dynamic Masking Embedded Indexing

**Ephemeral-Entropy Indexing for OS Dispatch Tables**

DMEI is an $O(1)$ secure indexing algorithm designed to protect operating system command dispatch tables (kernel syscall tables, interpreter opcode tables, and embedded handler routers) from memory-corruption and hardware-fault attacks. 

By utilizing **ephemeral-entropy indexing**—where storage positions depend on a cryptographic value destroyed immediately during construction—DMEI provides mathematical guarantees against write-only adversaries while maintaining the microsecond latency required for low-level system execution.

---

## 🛡️ Core Security Properties

DMEI is instantiated as a modified **Cuckoo Hash Table** and guarantees four critical properties:

1. **Inline Tamper Detection:** Achieves a tamper detection probability of $1 - 2^{-32}$ *without* the latency overhead of traditional Message Authentication Codes (MAC).
2. **Index Obfuscation:** A write-only adversary (e.g., utilizing a Rowhammer exploit) cannot locate a specific entry or handler address without a pre-existing read primitive.
3. **Deterministic $O(1)$ Lookup:** Worst-case lookup time is strictly $O(1)$, ensuring predictable execution for Real-Time Operating Systems (RTOS) and embedded control loops.
4. **Session Binding:** Binds dispatch tables to the current execution session with a success probability of $1 - 2^{-32}$, preventing cross-session replay or layout prediction.

## ⚡ Performance Metrics

DMEI is optimized for environments where lookup times must remain under 10ns.

* **Median Lookup Time:** 5.5 ns 
* **Relative Cost:** Only 1.53x the cost of a standard, unprotected Cuckoo table.
* **Security Overhead:** 8x faster than traditional MAC-authenticated alternative structures.

## 🎯 Threat Model

DMEI is specifically architected to defeat adversaries utilizing:
* **Rowhammer** and similar hardware-fault injections.
* **Limited Write Primitives** (Write-What-Where vulnerabilities) where the attacker attempts to overwrite a function pointer in a dispatch table to hijack control flow.
* Attacks attempting to bypass CFI (Control-Flow Integrity) by targeting the static predictability of standard arrays or unprotected hash tables.

---

## 🛠️ Build & Integration (C/C++)

DMEI is built for Linux and embedded RTOS environments. It requires a C++17 compliant compiler and standard build tools.

### Prerequisites
* `gcc` or `clang` (C++17+)
* `CMake` 3.10+
* Linux Kernel Headers (for kernel module integration)

### Building the Library
```bash
git clone [https://github.com/eyalbraun/dmei.git](https://github.com/eyalbraun/dmei.git)
cd dmei
mkdir build && cd build
cmake ..
make
