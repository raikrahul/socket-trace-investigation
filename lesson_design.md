---
layout: default
title: "00_DESIGN"
---

# 00. DESIGN: KERNEL ARCHITECTURE

## ABSTRACT
This document analyzes the structural prerequisites for `sock_init`. It derives the design choices of Linked Lists (Registration), Contexts (Configuration), and Co-allocation (Performance) from the source code.

## 1. THE REGISTRY (EXTENSIBILITY)

The kernel supports arbitrary filesystems via a runtime registration interface.

**Axiom 01**: `struct file_system_type` (`include/linux/fs.h`) contains `struct file_system_type * next;` (Offset 0x38).
**Implication**: The type itself is a node in a Singly Linked List.

**Axiom 02**: `static struct file_system_type *file_systems;` (`fs/filesystems.c`).
**Implication**: The kernel maintains a global head pointer for this list.

**Mechanism (Registration)**:
When `register_filesystem` is called:
1.  **Locking**: `write_lock(&file_systems_lock)`. Protects the global list.
2.  **Insertion**: `*p = fs;`. Appends the new type to the list.

## 2. THE CONTEXT (SEPARATION)

The VFS separates configuration from instantiation.

**Axiom 03**: `vfs_kern_mount` calls `fs_context_for_mount` (`fs/namespace.c`).
**Mechanism**: Allocates `struct fs_context`, a temporary scratchpad.

**Axiom 04**: `sockfs_init_fs_context` (`net/socket.c`).
**Implication**: The kernel delegates initialization.
- `ctx->ops = &sockfs_ops;`: Binds operations to the context.

## 3. THE TRANSIENT SUPERBLOCK (INSTANCE)

Every filesystem requires a Superblock (`struct super_block`) to manage inodes.

**Axiom 05**: `sget_fc` (`fs/super.c`) allocates `struct super_block`.
**Axiom 06**: `pseudo_fs_fill_super` (`fs/libfs.c`).
- `s->s_magic = SOCKFS_MAGIC;`.
- `s->s_op = ctx->ops;`.

**Conclusion**: The superblock is an on-demand instance created at boot, hydrated with operations from the Context.

## 4. CO-ALLOCATION (LOCALITY)

Sockets require both networking state (`struct socket`) and VFS state (`struct inode`).

**Axiom 07**: `sock_alloc_inode` (`net/socket.c`) is called by `alloc_inode`.
- **Allocation**: `kmem_cache_alloc(sock_inode_cachep, ...)` (Line 647).
- **Size**: 768 Bytes (128 Socket + 624 Inode + 16 Pad).

**Implication**: A single allocation creates both objects contiguously.
- **Benefit**: Zero cache pointer chases between socket and inode.

## 5. THE GLOBAL HANDLE (VISIBILITY)

`sockfs` is not mounted by users (no `/mnt/sock`). It requires an internal handle.

**Axiom 08**: `sock_mnt` (`net/socket.c`).
- Definition: `static struct vfsmount *sock_mnt __read_mostly;`.

**Mechanism**:
- `kern_mount` returns a `struct vfsmount *`.
- `sock_mnt` stores this pointer globally.

**Conclusion**: `sock_mnt` is the persistent anchor. When `socket()` is called, the kernel uses this handle to access the filesystem instance.
