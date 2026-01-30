---
layout: default
title: "Era III: Boot Time"
---

# ERA III: BOOT TIME (THE AWAKENING)
**"When kernel got booted"**

The machine powers on. The kernel loads. No user has logged in yet. The "Factory" must be built before a single "Product" (socket) can be made.

## 01. THE SLAB CREATION
The function `sock_init()` executes from the `.init.text` list.
It calls `kmem_cache_create` to setup the memory pool for our Design (Era I).

```c
// net/socket.c
sock_inode_cachep = kmem_cache_create("sock_inode_cache", 768, ...);
```

**The Reality Check**:
The memory manager sees 768 bytes (12 cache lines). It decides to add a 64-byte "Redzone" for debugging/tracking.
- Requested Size: 768 bytes.
- Actual Size: 832 bytes (13 cache lines).

**Result**: An empty swimming pool of 832-byte slots is ready and waiting.

## 02. THE MOUNT POINT
The logic needs a home in the VFS. `sock_init` calls `kern_mount`.
It creates an invisible mount point for `sockfs`.
```bash
$ cat /proc/filesystems | grep sockfs
nodev sockfs
```
This is the "Anchor" that allows `socket()` to find the `sockfs_ops` (from Era II).

## 03. THE PROTOCOL REGISTRATION
The function `inet_init()` executes.
It takes the empty `inetsw` buckets (from Era II) and fills them with live protocols.
- `inetsw[1]` (SOCK_STREAM) gets **TCP**.
- `inetsw[2]` (SOCK_DGRAM) gets **UDP**.

**Result**: The switchboard is wired.

---
**STATUS CHECK**:
The Factory is built. The Blueprints are frozen. The Switchboard is live.
But there are **ZERO** sockets in the system.
We are waiting for... YOU.

**[ NEXT: ERA IV - THE SOCKET CALL ](lesson_runtime.html)**
