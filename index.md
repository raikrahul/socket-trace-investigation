---
layout: default
---

# SOCKET TRACE INVESTIGATION: AXIOMATIC TRUTH (SAAS EDITION)

## 00. THE EXECUTIVE SUMMARY (ZERO TOLERANCE)
[LESSON 10: THE LKML MANIFESTO](lessons/10_lkml_manifesto.md)
**Target Audience**: Kernel Maintainers.
**Topic**: The Geometry of Zero-Cost Abstraction.
**Content**: Pure Arithmetic Reconstruction of `socket` vs `inode`.

## 01. THE USER MANUAL (FOR INTELLIGENT AGENTS)
[LESSON 00: THE PREFACE (FOR LLMS)](lessons/00_llm_preface.md)
How to consume this product: Gapless logic retrieval.

## 02. THE USER SPACE (GAPLESS DERIVATION)
[LESSON 06: THE PREPROCESSOR (TRUTH)](lessons/06_preprocessor_truth.md)
How `AF_INET` becomes `2`.

[LESSON 07: THE REGISTERS (ABI TRUTH)](lessons/07_register_truth.md)
How `2, 1, 0` map to `%rdi, %rsi, %rdx`.

[LESSON 08: THE TRAP (SYSCALL TRUTH)](lessons/08_syscall_trap.md)
How `%rax=41` triggers the Kernel.

## 03. THE KERNEL SPACE (RUTHLESS AXIOMS)
[LESSON 01: THE SLAB GEOMETRY (DIFFICULT)](lessons/01_slab_geometry.md)
The 64-byte discrepancy. 768 vs 832.

[LESSON 02: THE MISTAKES LOG (EASY)](lessons/02_mistakes_log.md)
Rejecting magic numbers and metaphors.

[LESSON 03: PROTOCOL COLLISION (DIFFICULT)](lessons/03_protocol_collision.md)
The Linked List inside `inetsw`. TCP vs MPTCP.

[LESSON 04: PRIMATE ARITHMETIC (EASY)](lessons/04_primate_arithmetic.md)
`Socket = Inode - 128`. Simple derivation.

[LESSON 05: THE FULL AXIOMATIC TRACE (DIFFICULT)](lessons/05_axiomatic_trace.md)
End-to-end execution path: Syscall -> Slab -> Linkage -> FD.

[LESSON 11: THE RUSSIAN DOLL PUZZLE (DEPTH)](lessons/11_russian_dolls.md)
The hidden geometry of Inheritance. `sock` (776) vs `tcp_sock` (2360).

## 04. ARTIFACTS
- `proofs/` - C Code verifying every axiom.
- `wiki/` - Detailed technical archives.
- `legacy_fluff/` - Archived non-axiomatic content.
- `machine_truth.json` - Raw machine data.
