---
layout: default
---

# SOCKET TRACE INVESTIGATION: AXIOMATIC TRUTH

## 00. MANIFESTO
01. No stories.
02. No adjectives.
03. Input -> Computation -> Output.
04. Every line is an axiom or derived from previous axioms.

## 01. THE MISTAKES LOG
[View Full Ruthless Report](wiki/Mistakes.md)

### MAJOR ERROR: Array Assumption
- WRONG: `inetsw[SOCK_STREAM] = TCP`
- AXOIM: `inetsw[1]` is a LIST HEAD.
- PROOF: Runtime probe found [TCP, MPTCP] in the list.

## 02. THE PUZZLES SOLVED

### A. THE HARDER PUZZLE (Protocol Collision)
[View Solution](wiki/The_Harder_Puzzle.md)
- `inetsw` is a Hash Table + Linked List.
- TCP (6) and MPTCP (262) coexist in Bucket 1.
- Kernel scans list linearly to find match.

### B. THE SLAB GEOMETRY PUZZLE (Memory Layout)
[View Solution](wiki/Slab_Geometry_Puzzle.md)
- `struct socket_alloc` = 768 bytes (12 cache lines).
- `sock_inode_cache` = 832 bytes (13 cache lines).
- Discovery: 64-byte Slab Overhead proved via probe/slabinfo delta.

## 03. AXIOMATIC FLOW (socket(2, 1, 0))
01. User calls `socket(2, 1, 0)`.
02. Kernel enters `__sys_socket`.
03. Kernel accesses `inetsw[1]`.
04. Kernel ITERATES linked list.
05. Node 1 (TCP): Protocol 6 != 0.
06. Wildcard check: User passed 0? YES.
07. Match! Select TCP.
08. Return FD 3.

## 04. ARTIFACTS
- `proofs/axiomatic_app.c`: Verifies File Descriptor logic.
- `proofs/axiomatic_probe.c`: Verifies struct offsets (24, 288).
- `proofs/axiomatic_probe_v2.c`: Verifies inetsw linked list.
- `proofs/axiomatic_probe_v3.c`: Verifies Slab sizes and Alignment.
