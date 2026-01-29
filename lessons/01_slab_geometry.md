# LESSON 1: THE SLAB GEOMETRY (DIFFICULT)

## 01. THE INPUT
Request: `sizeof(struct socket_alloc)`.
Layout:
- `struct socket` (128 bytes)
- `struct inode` (624 bytes)
- Total Payload: 752 bytes.

## 02. THE ALIGNMENT
Hardware Constraint: 64-byte Cache Lines.
Calculation: `ceil(752 / 64) * 64 = 768`.
Compiler Output: `sizeof(struct socket_alloc)` = 768 bytes.

## 03. THE DISCREPANCY
Kernel Telemetry (`/proc/slabinfo`):
`sock_inode_cache` object size = **832 bytes**.

Gap Analysis:
`832 - 768 = 64 bytes`.

## 04. THE CONCLUSION
The Slab Allocator enforces its own metadata overhead.
In this specific kernel configuration (6.14.0-37), the overhead is exactly **64 bytes** (1 Cache Line) per object.
**Axiom:** Physical footprint != Struct size.
