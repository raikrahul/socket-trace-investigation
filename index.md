---
layout: default
---

# SOCKET TRACE INVESTIGATION: AXIOMATIC TRUTH

## 00. MANIFESTO
01. No stories.
02. No adjectives.
03. Input -> Computation -> Output.
04. Every line is an axiom or derived from previous axioms.

## 01. THE CURRICULUM

### [LESSON 1: THE SLAB GEOMETRY (DIFFICULT)](lessons/01_slab_geometry.md)
The 64-byte discrepancy. 768 vs 832.

### [LESSON 2: THE MISTAKES LOG (EASY)](lessons/02_mistakes_log.md)
Rejecting magic numbers and metaphors.

### [LESSON 3: PROTOCOL COLLISION (DIFFICULT)](lessons/03_protocol_collision.md)
The Linked List inside `inetsw`. TCP vs MPTCP.

### [LESSON 4: PRIMATE ARITHMETIC (EASY)](lessons/04_primate_arithmetic.md)
`Socket = Inode - 128`. Simple derivation.

### [LESSON 5: THE FULL AXIOMATIC TRACE (DIFFICULT)](lessons/05_axiomatic_trace.md)
End-to-end execution path: Syscall -> Slab -> Linkage -> FD.

## 02. ARTIFACTS
- `proofs/` - C Code verifying every axiom.
- `wiki/` - Detailed technical archives.
- `legacy_fluff/` - Archived non-axiomatic content.
