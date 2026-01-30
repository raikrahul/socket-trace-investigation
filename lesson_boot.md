---
layout: default
title: "02_BOOT_TIME"
---

# 02. BOOT TIME: THE COMPLETE TRACE

## ABSTRACT
This document analyzes the complete execution path of `sock_init`, covering `net_sysctl_init`, `skb_init`, `register_filesystem`, and `kern_mount`. It connects memory allocations and architectural decisions to system stability constraints.

## 0. THE PRECURSOR: NET SYSCTL INIT

**Objective**: Trace `net_sysctl_init()` at Line 3273.
**Context**: `net/sysctl_net.c`.

```c
__init int net_sysctl_init(void)
{
    // 1. REGISTER ROOT "/proc/sys/net"
    net_header = register_sysctl_sz("net", empty, 0);
    // Op: Allocation of ctl_table_header.
    // Rationale: Creating an empty directory acts as a placeholder.
    //            This prevents race conditions where sub-sysctls (ipv4, core)
    //            try to register under "net" before it exists.

    // 2. REGISTER PERNET SUBSYS
    ret = register_pernet_subsys(&sysctl_pernet_ops);
    // Op: Adds 'sysctl_net_init' to the per-namespace initializer list.
    // Rationale: Linux supports Network Namespaces (Containers).
    //            Every time a new Namespace is created, 'sysctl_net_init' will fire,
    //            creating a private /proc/sys/net instance for that container.
}
```

## 1. THE FOUNDATION: SKBUFF INIT

**Objective**: Trace `skb_init()` at Line 3280.
**Context**: `net/core/skbuff.c` (Line 4905).

```c
void __init skb_init(void)
{
    // 1. SKBUFF HEAD CACHE
    skbuff_cache = kmem_cache_create_usercopy("skbuff_head_cache",
                          sizeof(struct sk_buff), ...);
    // Size: 224 bytes (approx, depends on config).
    // Rationale: The 'sk_buff' is the metadata wrapper for every packet.
    //            We need a dedicated, high-performance SLAB cache because
    //            networking creates/destroys millions of these per second.
    //            'SLAB_PANIC' ensures boot fails if we can't allocate this.

    // 2. FCLONE CACHE
    skbuff_fclone_cache = kmem_cache_create("skbuff_fclone_cache", ...);
    // Rationale: Fast Cloning. 
    //            Optimized allocator for cloned packets (multicast/sniffers),
    //            allocating pairs of sk_buffs efficiently.
}
```

## 2. FILESYSTEM REGISTRATION

**Objective**: Trace `register_filesystem(&sock_fs_type)` at Line 3288.
**Context**: `fs/filesystems.c`.

```c
int register_filesystem(struct file_system_type * fs)
{
    // 1. INPUT ANALYSIS
    // Ptr: 0xffffffff839c2dc0 (sock_fs_type)
    // Why: This structure contains the 'blueprints' (methods) for the socket filesystem.

    struct file_system_type ** p;

    // 2. SYNTAX VALIDATION
    BUG_ON(strchr(fs->name, '.')); 
    // Op: Scan "sockfs" for '.'.
    // Rationale: The VFS uses specific syntax like "ext4.usrquota" for subtypes.
    //            Allowing dots in the base name would make parsing ambiguous.
    // Consequence: A kernel panic prevents corrupted filesystem namespaces.

    // 3. STATE VALIDATION
    if (fs->next) 
        return -EBUSY;
    // Op: Check offset 0x38 (56).
    // Rationale: A filesystem type can only be in one list at a time.
    //            Non-NULL 'next' implies it's already registered or corrupted.

    // 4. CONCURRENCY CONTROL
    write_lock(&file_systems_lock);
    // Address: 0xffffffff84167a58.
    // Rationale: The 'file_systems' list is a global singleton resource.
    //            During boot (or module load), multiple CPUs triggers registration.
    //            Exclusive write access prevents list corruption (dangling pointers).

    // 5. DUPLICATE DETECTION
    p = find_filesystem(fs->name, strlen(fs->name));
    // Logic: O(N) Linear Scan.
    // Rationale: We must ensure uniqueness of the filesystem name ("sockfs").
    //            Collision would make `mount -t sockfs` non-deterministic.

    // 6. ATOMIC INSERTION
    if (*p)
        res = -EBUSY;
    else
        *p = fs;
    // Op: Write 0xffffffff839c2dc0 to the tail pointer.
    // Rationale: Changing the pointer is the "Commit Point".
    //            Done under lock to ensure atomicity.

    // 7. RELEASE
    write_unlock(&file_systems_lock);
    // Rationale: Minimize critical section duration to reduce contention.

    return 0;
}
```

### MEMORY STATE: REGISTRY
```ascii
[ .data SECTION ]
0xffffffff84167a60 (file_systems)
+------------------------+
| POINTER: 0xffffffff... | --> Points to sock_fs_type
+------------------------+
           |
           v
[ .data SECTION ]
0xffffffff839c2dc0 (sock_fs_type)
+------------------------+ <--- 0x00
| name: "sockfs"         |
+------------------------+ <--- 0x08
| fs_flags: 0x00000000   |
+------------------------+ ...
+------------------------+ <--- 0x38 (Offset 56)
| next: NULL             |
+------------------------+
```

## 3. MOUNT POINT ANCHORING

**Objective**: Trace `sock_mnt = kern_mount(&sock_fs_type)` at Line 3291.
**Context**: `fs/namespace.c`.

```c
struct vfsmount *vfs_kern_mount(struct file_system_type *type, ...)
{
    // 1. CONTEXT INSULATION
    fc = fs_context_for_mount(type, flags);
    // Op: Heap Alloc (fs_context) at 0xffff888100100000.
    // Rationale: Modern Linux separates "configuration" (fs_context) from "action" (mount).
    //            This allows parameters to be validated *before* touching the superblock.
    
    // 2. MOUNT EXECUTION
    mnt = fc_mount(fc);
    
        // A. INSTANCE CREATION
        // Call: alloc_super -> Heap Alloc 0xffff888100200000.
        // Rationale: The 'sock_fs_type' is static code (Class).
        //            The 'super_block' is the runtime instance (Object).
        //            We need an object to hold the list of active inodes.

        // B. ROOT CREATION
        // Call: d_make_root -> Heap Alloc 0xffff888100300000.
        // Rationale: Every filesystem needs an entry point (Root Directory).
        //            Even though sockfs is invisible, it must adhere to VFS topology.

    // 3. HANDLE CREATION
    // Heap Alloc 0xffff888100400000 (vfsmount).
    // Link: mnt->mnt_sb = 0xffff888100200000.
    // Rationale: Checkpointing.
    //            'sock_mnt' is the permanent handle. The kernel stores it globally
    //            so it never has to re-mount 'sockfs' again.
    //            It is the "Anchor" that keeps the superblock alive.

    return mnt;
}
```

### MEMORY STATE: ANCHOR
```ascii
[ .data SECTION ]
0xffffffff83a767a0 (sock_mnt)
+--------------------------+
| PTR: 0xffff888100400000  | ----------------+
+--------------------------+                 |
                                             |
           [ HEAP: vfsmount ] <--------------+
           0xffff888100400000
           +--------------------------+ <--- 0x00
           | mnt_root: 0x...300000    | ------------+
           +--------------------------+ <--- 0x08   |
           | mnt_sb:   0x...200000    | --+         |
           +--------------------------+   |         |
                                          |         |
           [ HEAP: super_block ] <--------+         |
           0xffff888100200000                       |
           +--------------------------+             |
           | s_type: 0xffffffff839c2...             |
           +--------------------------+             |
                                                    |
           [ HEAP: dentry (Root) ] <----------------+
           0xffff888100300000
           +--------------------------+
           | d_inode: 0x... (Inodes)  |
           +--------------------------+
```

## 4. ALGORITHMIC REASONING

**Problem**: Is a Linear Scan O(N) acceptable for filesystem registration?

**Calculations**:
- **N = 1000** filesystems (Extreme Case).
- **Comparison Cost**: 5ns (L1 Cache Hit).
- **Total Ops**: sum(1..N) = 500,500.
- **Time**: ~2.5 ms.

**Reasoning**:
1.  **Frequency**: This happens only *once* per filesystem type (boot or module load).
2.  **Scale**: Real systems have < 50 types (ext4, xfs, proc, sysfs, overlay, etc.).
3.  **Trade-off**: Implementing a Hash Map for 50 items introduces overhead (allocation, hashing) and complexity that outweighs the saving of ~2.5ms at boot.
4.  **Conclusion**: The "naive" O(N) approach is mathematically optimal for the constraints and scale of the problem.

**[ NEXT: 03_RUNTIME ](lesson_runtime.html)**
