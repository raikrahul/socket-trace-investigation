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

## 02. THE HARDER PUZZLE
[View Axiomatic Derivation](wiki/The_Harder_Puzzle.md)

### DATA DUMP (Kernel 6.14.0-37-generic)
address(inetsw): 0xffffffffb1bcd040
inetsw[1] contains:
  1. TCP (proto=6)
  2. MPTCP (proto=262)

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
- `axiomatic_app.c`: Verifies File Descriptor logic.
- `axiomatic_probe.c`: Verifies struct offsets (24, 288).
- `axiomatic_probe_v2.c`: Verifies inetsw linked list (The Harder Puzzle).
