# LESSON 3: PROTOCOL COLLISION (DIFFICULT)

## 01. THE PROBLEM
Input: `socket(AF_INET, SOCK_STREAM, 0)`.
Candidate 1: TCP (Type 1, Protocol 6).
Candidate 2: MPTCP (Type 1, Protocol 262).
Both are registered in `inetsw[1]`.

## 02. THE MECHANISM
Structure: Array of Linked Lists (Hash Table).
`inetsw[1]` -> [Node A] -> [Node B] -> NULL.

## 03. THE EXECUTION TRACE
1. Kernel iterates list at `inetsw[1]`.
2. Inspects Node A (TCP):
   - `protocol` match? (6 vs 0). NO.
   - `IPPROTO_IP` match? (0 vs 0). YES.
   - Permitted? YES (PERMANENT flag).
3. **Selection: TCP**.

## 04. THE PROOF
Probe `axiomatic_probe_v2` dumped the physical list:
- Node 1: TCP (6)
- Node 2: MPTCP (262)
**Axiom:** Protocol selection is a linear scan of a collision chain.
