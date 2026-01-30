---
layout: default
title: "01_COMPILE_TIME"
---

# 01. COMPILE TIME: ELF ARTIFACT ANALYSIS

## ABSTRACT
This document analyzes the link-time layout of the Socket Subsystem. It verifies the placement of critical symbols (`inetsw`, `sockfs_ops`, `sock_init`) within the ELF sections (`.bss`, `.rodata`, `.init.text`) of the `vmlinux` binary.

## AXIOM 01: THE BINARY ARTIFACT
- **Kernel Image**: `/boot/vmlinux-6.14.0-37-generic`
- **Symbol Map**: `/boot/System.map-6.14.0-37-generic`

## SECTION 01: THE BSS SECTION (UNINITIALIZED DATA)

### Mechanism: Static Allocation
The `inetsw` array serves as the protocol registry. It is defined as a static global variable without an initializer.

**Source Axiom**:
```c
// net/ipv4/af_inet.c
static struct list_head inetsw[SOCK_MAX];
```

**Linker Artificat**:
- **Symbol**: `inetsw`
- **Address**: `ffffffff841cd040`
- **Section**: `.bss`

**Derivation**:
The symbol resides in the BSS because it is zero-initialized at load time. The address `ffffffff841cd040` is assigned by the linker script (`vmlinux.lds.S`), placing it in the high-memory kernel data segment. No runtime allocation is required; the memory is reserved by the ELF loader.

## SECTION 02: THE RODATA SECTION (READ-ONLY DATA)

### Mechanism: Constant Protection
The `sockfs_ops` structure defines the filesystem methods. It is marked `const` to enforce immutability.

**Source Axiom**:
```c
// net/socket.c
static const struct super_operations sockfs_ops = { ... };
```

**Linker Artifact**:
- **Symbol**: `sockfs_ops`
- **Address**: `ffffffff8276e740`
- **Section**: `.rodata`

**Derivation**:
The `const` qualifier directs `gcc` to emit this symbol into the `.rodata` section. The memory protection hardware (CR0.WP bit) prevents accidental or malicious modification of these function pointers at runtime.

## SECTION 03: THE INIT.TEXT SECTION (DISCARDABLE CODE)

### Mechanism: Boot-Time Memory Reclaim
The `sock_init` function is required only during the boot sequence.

**Source Axiom**:
```c
// net/socket.c
static int __init sock_init(void) { ... }
```

**Linker Artifact**:
- **Symbol**: `sock_init`
- **Address**: `ffffffff83b9bbb0`
- **Section**: `.init.text`

**Derivation**:
The `__init` macro expands to `__section(".init.text")`. The linker groups all such functions together. Once `start_kernel` completes, the function `free_initmem` releases the entire page range containing `ffffffff83b9bbb0` to the page allocator.

## SECTION 04: THE DATA SECTION (PERSISTENT STATE)

### Mechanism: Global Visibility
The `sock_mnt` pointer anchors the socket filesystem instance.

**Source Axiom**:
```c
// net/socket.c
static struct vfsmount *sock_mnt;
```

**Linker Artifact**:
- **Symbol**: `sock_mnt`
- **Address**: `ffffffff83a767a0`
- **Section**: `.data` (Read-Mostly)

**Derivation**:
This variable persists for the kernel's lifetime. Its address `ffffffff83a767a0` is the fixed location where the `kern_mount` result will be stored.

## SECTION 05: INLINE OPTIMIZATION

### Mechanism: Compile-Time Offset Calculation
The `SOCKET_I` macro resolves the `struct socket` from a `struct inode`.

**Source Logic**:
```c
return &container_of(inode, struct socket_alloc, vfs_inode)->socket;
```

**Instruction Generation**:
Given:
- `sizeof(struct socket) = 128`
- `offsetof(struct socket_alloc, vfs_inode) = 128`

The compiler generates:
```asm
lea -128(%reg), %reg
```
There is no runtime overhead or complex logic. The relationship is strictly geometric and resolved during compilation.

**[ NEXT: 02_BOOT_TIME ](lesson_boot.html)**
