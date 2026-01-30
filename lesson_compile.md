---
layout: default
title: "01_COMPILE_TIME"
---

# 01. COMPILE TIME: ELF ARTIFACTS

This document traces critical socket symbols (`inetsw`, `sockfs_ops`, `sock_init`) to their ELF sections in the `vmlinux` binary.

**Binary**: `/boot/vmlinux-6.14.0-37-generic`

## 1. BSS (UNINITIALIZED DATA)

The `inetsw` array is the protocol registry. It is defined as a static global variable without an initializer.

**Code**:
```c
// net/ipv4/af_inet.c
static struct list_head inetsw[SOCK_MAX];
```

**Artifact**:
- **Symbol**: `inetsw`
- **Address**: `ffffffff841cd040`
- **Section**: `.bss`

**Trace**:
The symbol is zero-initialized at load time. Its address is assigned by the linker script (`vmlinux.lds.S`). No runtime allocation is performed; the memory is reserved by the ELF loader.

## 2. RODATA (READ-ONLY DATA)

The `sockfs_ops` structure defines filesystem methods.

**Code**:
```c
// net/socket.c
static const struct super_operations sockfs_ops = { ... };
```

**Artifact**:
- **Symbol**: `sockfs_ops`
- **Address**: `ffffffff8276e740`
- **Section**: `.rodata`

**Trace**:
The `const` qualifier directs GCC to emit this symbol into `.rodata`. The hardware (CR0.WP) enforces immutability, preventing runtime modification.

## 3. INIT.TEXT (DISCARDABLE CODE)

The `sock_init` function is required only during boot.

**Code**:
```c
// net/socket.c
static int __init sock_init(void) { ... }
```

**Artifact**:
- **Symbol**: `sock_init`
- **Address**: `ffffffff83b9bbb0`
- **Section**: `.init.text`

**Trace**:
The `__init` macro expands to `__section(".init.text")`. Once `start_kernel` completes, `free_initmem` releases this page range to the free pool.

## 4. DATA (PERSISTENT STATE)

The `sock_mnt` pointer anchors the filesystem instance.

**Code**:
```c
// net/socket.c
static struct vfsmount *sock_mnt;
```

**Artifact**:
- **Symbol**: `sock_mnt`
- **Address**: `ffffffff83a767a0`
- **Section**: `.data`

**Trace**:
This variable persists for the kernel's lifetime due to its read-write nature (it is written once at boot).

## 5. INLINE MATH (OPTIMIZATION)

The `SOCKET_I` macro resolves the `struct socket` from `struct inode`.

**Code**:
```c
return &container_of(inode, struct socket_alloc, vfs_inode)->socket;
```

**Trace**:
- `sizeof(struct socket) = 128`
- `offsetof(struct socket_alloc, vfs_inode) = 128`

The compiler generates a geometric offset calculation: `lea -128(%reg), %reg`. There is no runtime API call.
