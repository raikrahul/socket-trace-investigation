---
layout: default
title: "00_BOOT_TRACE"
---

# 00. BOOT TRACE: THE INITIALIZATION

## ABSTRACT
This document analyzes the architectural primitives required to construct the Linux Socket Subsystem (`sock_init`). It derives the design choices of **Dynamic Extensibility** (via Linked Lists), **Separation of Concern** (via Contexts), and **Cache Locality** (via Co-allocation) directly from the kernel source axioms.

## ARCHITECTURAL PRIMITIVE 01: THE REGISTRY (EXTENSIBILITY)

### Design Choice I: Dynamic Registration
The kernel must support an arbitrary number of filesystems without recompilation. This necessitates a runtime registration interface.

**Axiom 01**: `struct file_system_type` (`include/linux/fs.h`, Line 2450) contains `struct file_system_type * next;` (Line 2466).
**Design Implication**: The type itself is designed to be a node in a Singly Linked List.

**Axiom 02**: `static struct file_system_type *file_systems;` (`fs/filesystems.c`, Line 34).
**Design Implication**: The kernel maintains a global head pointer for this list.

**Mechanism (Registration)**:
When `sock_init` calls `register_filesystem`:
- **Locking**: `write_lock(&file_systems_lock)` (Line 84). The design chooses a Read-Write lock, prioritizing parallel reads (lookups) over writes (registrations).
- **Insertion**: `*p = fs;` (Line 89). The new type is appended.

**Conclusion**: The socket subsystem is not "special"; it is simply a node in a dynamic list.

## ARCHITECTURAL PRIMITIVE 02: THE CONTEXT (SEPARATION)

### Design Choice II: Configuration vs. Instance
The VFS separates "how to configure a mount" from "the mount itself". This allows parameters to be validated before memory is fully committed.

**Axiom 03**: `vfs_kern_mount` calls `fs_context_for_mount` (`fs/namespace.c`, Line 1145).
**Mechanism**: This allocates a `struct fs_context`—a temporary scratchpad.

**Axiom 04**: `sockfs_init_fs_context` (`net/socket.c`, Line 412).
**Design Implication**: The kernel delegates initialization to the specific subsystem.
- **Action**: `ctx->ops = &sockfs_ops;`. This binds the *Behavior* (Operations) to the *Context*.

**Conclusion**: The "Context" acts as a factory blueprint, isolating the definition of operations from their execution.

## ARCHITECTURAL PRIMITIVE 03: THE TRANSIENT SUPERBLOCK (INSTANCE)

### Design Choice III: The Transient Factory
Every filesystem instance needs a "Superblock" to manage its inodes. For sockets, this superblock is invisible and transient.

**Axiom 05**: `sget_fc` (`fs/super.c`) allocates `struct super_block`.
**Axiom 06**: `pseudo_fs_fill_super` (`fs/libfs.c`).
- **Action**: `s->s_magic = SOCKFS_MAGIC;`.
- **Action**: `s->s_op = ctx->ops;`.
**Design Implication**: The superblock is "hydrated" with the operations from the Context (from Axiom 04).

**Conclusion**: The superblock is the "Factory" that will produce all future sockets. It is created on-demand during boot.

## ARCHITECTURAL PRIMITIVE 04: THE CENTAUR (LOCALITY)

### Design Choice IV: Co-allocation for Performance
Sockets require both networking state (`struct socket`) and VFS state (`struct inode`). Allocating them separately would double the cache misses.

**Axiom 07**: `sock_alloc_inode` (`net/socket.c`) is called by `alloc_inode`.
- **Allocation**: `kmem_cache_alloc(sock_inode_cachep, ...)` (Line 647).
- **Size**: 768 Bytes (Derived: 128 Socket + 624 Inode + 16 Pad).

**Design Implication**: Steps 05/06 create a factory (`super_block`) whose sole purpose is to dispense these 768-byte "Centaur" objects.
- **Benefit**: One allocation, one free, zero cache pointer chases between socket and inode.

## ARCHITECTURAL PRIMITIVE 05: THE ANCHOR (VISIBILITY)

### Design Choice V: The Global Handle
Since `sockfs` is never mounted by a user (e.g., `mount -t sockfs`), it has no path in the namespace (like `/tmp`). It needs an internal handle.

**Axiom 08**: `sock_mnt` (`net/socket.c`, Line 423).
- **Definition**: `static struct vfsmount *sock_mnt __read_mostly;`.
- **Scope**: Global variable.

**Mechanism**:
- `kern_mount` returns a `struct vfsmount *` (Line 1160 `fs/namespace.c`).
- `sock_mnt = ...` (Line 2950 `net/socket.c`).

**Conclusion**: This variable is the *only* link between the kernel's generic code and the hidden socket universe. When a user calls `socket()`, the kernel reads `sock_mnt` to find the superblock (Primitive 03) to ask the factory to create a Centaur (Primitive 04).
