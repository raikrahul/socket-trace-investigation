---
layout: default
title: "02_BOOT_TIME"
---

# 02. BOOT TIME: SYSTEM INITIALIZATION

This document traces the execution path of `sock_init`, covering `net_sysctl_init`, `skb_init`, `register_filesystem`, and `kern_mount`.

## 1. NET SYSCTL INIT

**Objective**: Trace `net_sysctl_init()` (Line 3273 `net/socket.c`).
**Context**: `net/sysctl_net.c`.

```c
__init int net_sysctl_init(void)
{
    // 1. REGISTER ROOT "/proc/sys/net"
    net_header = register_sysctl_sz("net", empty, 0);
    // Purpose: Creates the directory placeholder.
    // Prevents race conditions during sub-system registration.

    // 2. REGISTER PERNET SUBSYS
    ret = register_pernet_subsys(&sysctl_pernet_ops);
    // Purpose: Hooks 'sysctl_net_init' into Namespace creation.
    // Ensures every new container gets its own /proc/sys/net.
}
```

## 2. SKBUFF INIT

**Objective**: Trace `skb_init()` (Line 3280 `net/socket.c`).
**Context**: `net/core/skbuff.c`.

```c
void __init skb_init(void)
{
    // 1. SKBUFF HEAD CACHE
    skbuff_cache = kmem_cache_create_usercopy("skbuff_head_cache",
                          sizeof(struct sk_buff), ...);
    // Size: ~224 bytes.
    // Purpose: Metadata wrapper for network packets.
    // Critical Path: Must succeed (SLAB_PANIC) or kernel halts.

    // 2. FCLONE CACHE
    skbuff_fclone_cache = kmem_cache_create("skbuff_fclone_cache", ...);
    // Purpose: Optimized allocator for cloned packets.
}
```

## 3. FILESYSTEM REGISTRATION

**Objective**: Trace `register_filesystem(&sock_fs_type)` (Line 3288 `net/socket.c`).
**Context**: `fs/filesystems.c`.

```c
int register_filesystem(struct file_system_type * fs)
{
    // 1. INPUT
    // Ptr: 0xffffffff839c2dc0 (sock_fs_type)

    struct file_system_type ** p;

    // 2. VALIDATION
    BUG_ON(strchr(fs->name, '.')); 
    // Purpose: Dots are reserved for subtype syntax.

    if (fs->next) 
        return -EBUSY;
    // Purpose: Enforce single-list membership.

    // 3. LOCKING
    write_lock(&file_systems_lock);
    // Address: 0xffffffff84167a58.
    // Purpose: Global list protection.

    // 4. DUPLICATE CHECK (Linear Scan)
    p = find_filesystem(fs->name, strlen(fs->name));
    if (*p)
        res = -EBUSY;
    else
        *p = fs; // 5. INSERTION

    write_unlock(&file_systems_lock);
    return 0;
}
```

### MEMORY MAP: FILE_SYSTEMS
```ascii
[ .data SECTION ]
0xffffffff84167a60 (file_systems)
+------------------------+
| POINTER: 0xffffffff... | --> Points to sock_fs_type
+------------------------+
           |
           v
0xffffffff839c2dc0 (sock_fs_type)
+------------------------+ <--- 0x00
| name: "sockfs"         |
+------------------------+ ...
+------------------------+ <--- 0x38 (Offset 56)
| next: NULL             |
+------------------------+
```

## 4. MOUNT POINT CREATION

**Objective**: Trace `sock_mnt = kern_mount(&sock_fs_type)` (Line 3291 `net/socket.c`).
**Context**: `fs/namespace.c`.

```c
struct vfsmount *vfs_kern_mount(struct file_system_type *type, ...)
{
    // 1. ALLOCATE CONTEXT
    fc = fs_context_for_mount(type, flags);
    // Heap: 0xffff888100100000 (fs_context).
    
    // 2. MOUNT
    mnt = fc_mount(fc);
    
        // A. CREATE SUPERBLOCK (Instance)
        // Call: alloc_super -> Heap: 0xffff888100200000.

        // B. CREATE ROOT DENTRY
        // Call: d_make_root -> Heap: 0xffff888100300000.

    // 3. CREATE HANDLE (vfsmount)
    // Heap: 0xffff888100400000.
    // Link: mnt->mnt_sb = 0xffff888100200000.
    
    // Purpose: 'sock_mnt' is the global handle keeping the superblock alive.
    return mnt;
}
```

### MEMORY MAP: SOCK_MNT
```ascii
0xffffffff83a767a0 (sock_mnt)
+--------------------------+
| PTR: 0xffff888100400000  | ----------------+
+--------------------------+                 |
                                             |
           [ HEAP: vfsmount ] <--------------+
           0xffff888100400000
           | mnt_sb:   0x...200000    | --+
                                          |
           [ HEAP: super_block ] <--------+
           0xffff888100200000
```

## 5. ALGORITHMIC ANALYSIS

**Question**: Is O(N) linear scan acceptable?

**Data**:
- **N**: < 50 types (ext4, xfs, etc).
- **Cost**: < 500 comparison ops.
- **Time**: Negligible (~microseconds).

**Conclusion**: Linear scan is optimal for small, read-mostly datasets.
