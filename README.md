# DMEI — Dynamic Memory Encryption and Indirection

DMEI is an educational research proof of concept exploring a moving-target dispatch architecture for hardware-register-bound secrets and indirect function resolution.

> **Current status:** This repository is a small, standalone POC. It is not a production security mechanism, operating-system component, compiler, hypervisor or complete control-flow-integrity implementation.

## Concept

The POC explores a dispatch design that combines:

- epoch-dependent table rebuilding;
- encrypted indirection values;
- simulated caller-IP validation;
- function-address integrity values; and
- reconstruction of an indirect target from an anchor, an offset and an epoch-dependent value.

The implementation is intended to make the data flow of the concept inspectable. It does not establish that the design defeats memory disclosure, static analysis, control-flow hijacking or return-oriented programming in a real system.

## What is implemented

The current program contains three hard-coded demonstration functions, a fixed-size dispatch table, simulated caller addresses, epoch-dependent demo values, an FMAC-style integrity calculation and a lookup path that prints verification results.

The POC deliberately does not perform the final indirect jump. It prints whether the integrity calculation succeeded and labels the result as a demonstration.

## What is not implemented

The following capabilities are outside the scope of the current code:

- production-grade AES encryption and decryption;
- authenticated encryption or a cryptographic proof of integrity;
- hardware-isolated register secrets;
- real caller-IP instrumentation;
- compiler, linker or operating-system integration;
- a complete ROP or CFI defense;
- secure key provisioning and lifecycle management;
- resistance evaluation against memory snapshots or static analysis; and
- a live indirect dispatch mechanism.

The demo values are deterministic and the program prints internal state for educational inspection. These properties are useful for learning but are not suitable for a deployed security design.

## Important implementation note

The current `enc_E()` and `dec_E()` helpers are demonstration transformations based on AES-NI intrinsics. They are not a complete, standards-compliant reversible AES encryption/decryption pair. Do not reuse them as cryptography.

## Build

A compiler with x86-64 AES-NI support and the relevant intrinsics is required. For example:

```shell
gcc -O2 -maes -msse4.1 -Wall -Wextra -pedantic dmei_POC.c -o dmei_poc
./dmei_poc
```

Run the program only on a machine where AES-NI is available. The output shows an initial table build, valid and invalid caller demonstrations, and per-epoch state.

## Suggested experiments

Useful educational experiments include changing the simulated caller address, modifying a table entry, observing FMAC failure, comparing epochs and documenting which invariants are preserved after rebuilding the table.

A future research version should define a threat model, use standard cryptographic constructions, avoid printing secrets, separate test keys from production keys, add property-based tests and compare the design against established control-flow-integrity and memory-encryption techniques.

## Security disclaimer

This repository is provided for education and research discussion. It must not be used to protect real secrets, execute privileged code or secure safety-critical systems.

## License

See `LICENSE` for the repository license. The code and documentation are provided without a claim of production security.
