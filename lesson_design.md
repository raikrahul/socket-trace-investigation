---
layout: default
title: "Era I: Design Time"
---

# ERA I: DESIGN TIME (THE ARCHITECT'S LEDGER)
**"Way before you wrote socket"**

Before a single line of kernel code was compiled, the architects had to solve a geometric paradox. They had to bridge two incompatible worlds with zero cost.

## 01. THE TWO WORLDS
1.  **VFS (File System)**: Speaks `struct inode`. Handles permissions, ownership, disks.
    - Size: 624 Bytes.
2.  **NET (Networking)**: Speaks `struct socket`. Handles TCP states, buffers, connections.
    - Size: 128 Bytes.

## 02. THE PARADOX
When a user calls `socket()`, the kernel must return a **File Descriptor** (VFS).
- If VFS owns the object, how does NET access it? (Pointer Chase? Slow).
- If NET owns the object, how does VFS access it? (Pointer Chase? Slow).

## 03. THE GEOMETRIC SOLUTION (THE CENTAUR)
The architects chose **Co-location**. They defined a new structure `socket_alloc` that contains *both*.

```c
// Validated Layout (v6.14.0-37)
struct socket_alloc {
    struct socket socket;   // Offset 0    (128 bytes)
    struct inode vfs_inode; // Offset 128  (624 bytes)
};
```

**Total Size**: 128 + 624 = 752 Bytes.
**Aligned Size**: 768 Bytes (padded for cache alignment).

## 04. THE CONSEQUENCE
This design choice froze the "Truth" of the kernel long before compilation.
It dictates that:
1.  Every socket is an inode.
2.  Every socket-inode is exactly 768 bytes (12 Cache Lines).
3.  The conversion between them is simple arithmetic: `Address +/- 128`.

## 05. PROOF OF DESIGN
```bash
$ pahole -C socket_alloc vmlinux
struct socket_alloc {
    struct socket              socket;               /*     0   128 */
    struct inode               vfs_inode;            /*   128   624 */
    /* size: 768, cachelines: 12, padding: 16 */
};
```

**[ NEXT: ERA II - COMPILE TIME ](lesson_compile.html)**
