DMEI: Dynamic Memory Encryption and Indirection
A Proof‑of‑Concept implementation of a moving‑target dispatch layer for hardware‑register‑bound secrets.

Overview
DMEI is a defense‑in‑depth dispatch architecture designed to prevent address resolution, forward‑edge hijacking, and code‑page tampering — even under full RAM disclosure. By combining AES‑encrypted indirection, moving‑target reshuffling, and per‑lookup function‑body integrity, DMEI ensures that memory snapshots, static analysis, and pointer harvesting yield no actionable information.

The core idea: all secrets live exclusively in CPU registers, never in RAM, and the dispatch table is re‑encrypted and reshuffled every epoch.

Core Features
Two‑Hop AES Indirection  
Dispatch resolution requires two independent AES‑derived indexes keyed by hardware‑resident secrets. Secrets (K, T, E) never leave the register file.

Moving‑Target Reshuffle  
Every epoch rotates all secrets, offsets, and slot positions. Any memory snapshot becomes obsolete within milliseconds.

Function‑Body Integrity (FMAC)  
Each dispatch verifies a keyed MAC over the first bytes of the function body and its canonical address. Code tampering or address redirection is detected immediately.

Caller IP Validation  
Each function can only be invoked from authorized call sites. Unauthorized callers trigger reshuffle and abort.

Encrypted Return‑Address Binding (ROP Defense)  
Return addresses are shadowed with encrypted counterparts. Any mismatch indicates tampering and halts execution.

Prerequisites
Architecture: x86‑64

Hardware Support: AES‑NI required for constant‑time encryption

Compiler: GCC or Clang with wmmintrin.h and immintrin.h support

Quick Start
Compile the PoC with:

bash
gcc -O3 -Wall -maes -mavx -mavx2 -o dmei_POC dmei_POC.c
Run the binary:

bash
./dmei_POC
Observe:

Epoch rotation

Table reshuffling

FMAC integrity verification

Caller IP validation

Offset reconstruction

Warning
This is a research‑grade Proof of Concept.
It assumes:

A trusted Ring‑0 entity for initial key generation

Hardware‑level guarantees that CPU registers cannot be leaked

No speculative execution attacks leaking register contents

It is not intended for production use.
