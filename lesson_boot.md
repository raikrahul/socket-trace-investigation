---
layout: default
title: "02_BOOT_TIME"
---

# 02. BOOT TIME: EXECUTION TRACE

## ABSTRACT
This document provides an instruction-level trace of the `sock_init` execution sequence. It unwinds the caller stack from `kernel_init` and documents the state transitions of the memory management and VFS subsystems.

## SECTION 01: THE INITCALL MECHANISM

The `sock_init` function is invoked via the generic `do_initcalls` loop. It is not called explicitly by name in the source code.

**Stack Unwinding (`init/main.c`)**:
1.  **Line 1558**: `kernel_init_freeable()` -> `do_basic_setup()`.
2.  **Line 1340**: `do_basic_setup()` -> `do_initcalls()`.
3.  **Line 1321**: `do_initcalls()` iterates `initcall_levels`.
4.  **Line 1305**: `do_initcall_level()` dereferences the function pointer.
5.  **Line 1243**: `do_one_initcall()` executes `ret = fn()`.

**Runtime State**:
- **Instruction Pointer**: `ffffffff83b9bbb0` (`sock_init`).
- **Caller Address**: `ffffffff811f56a0` (`do_one_initcall`).

## SECTION 02: EXECUTION TRACE TABLE

| # | Call | Values | Data | Work | Errors | Real Value | Real Data | Caller | Current | Resumed |
|---|---|---|---|---|---|---|---|---|---|---|
| 01 | `sock_init()` | `void` | `init_net` initialized | Stack Frame Setup | - | `0xffffffff83b9bbb0` | `ret=0` | 1243 | 3267 | - |
| 02 | `init_inodecache()` | `void` | - | Jumps to Line 339 | - | - | - | 3286 | 3286 | - |
| 03 | `kmem_cache_create()` | `"sock_inode_cache"` | `size=768` | Allocates Slab Cache | - | `0xffff888100042400` | `cachep` | 3286 | 341 | - |
| 04 | `BUG_ON()` | `cachep == NULL` | `0 == 0` | Check Success | - | `false` | - | 3286 | 348 | - |
| 05 | Return | `void` | - | Stack Pop | - | - | - | 3286 | 349 | 3286 |

**State Change (Memory)**:
- A new Slab Cache (`sock_inode_cache`) is registered.
- Object Size: 768 bytes (Aligned to 832 bytes).

## SECTION 03: FILESYSTEM REGISTRATION

| # | Call | Values | Data | Work | Errors | Real Value | Real Data | Caller | Current | Resumed |
|---|---|---|---|---|---|---|---|---|---|---|
| 06 | `register_filesystem()` | `&sock_fs_type` | `name="sockfs"` | Locks `file_systems_lock` | - | `0xffffffff817f6c00` | `ret=0` | 3288 | 3288 | - |
| 07 | Check Error | `err` | `0` | Branch Not Taken | - | `0` | - | 3288 | 3289 | - |

**State Change (VFS)**:
- The `sock_fs_type` node is appended to the `file_systems` global list.

## SECTION 04: MOUNT POINT ANCHORING

| # | Call | Values | Data | Work | Errors | Real Value | Real Data | Caller | Current | Resumed |
|---|---|---|---|---|---|---|---|---|---|---|
| 08 | `kern_mount()` | `&sock_fs_type` | - | Creates VFS Mount | - | `0xffffffff817fa260` | `mnt` | 3291 | 3291 | - |
| 09 | Assignment | `sock_mnt = ...` | `mnt struct` | Writes Global Var | - | `0xffffffff83a767a0` | `sock_mnt` | 3291 | 3291 | - |
| 10 | `IS_ERR()` | `mnt` | `ptr > -4095` | Check Success | - | `false` | - | 3291 | 3292 | - |

**State Change (Globals)**:
- `sock_mnt` (Address `ffffffff83a767a0`) now points to a valid `vfsmount` structure.
- This provides the handle for subsequent `socket(2)` syscalls.

## SECTION 05: FUNCTION EXIT

| # | Call | Values | Data | Work | Errors | Real Value | Real Data | Caller | Current | Resumed |
|---|---|---|---|---|---|---|---|---|---|---|
| 11 | `out:` | `err` | `0` | Label Reached | - | `0` | - | - | 3275 | - |
| 12 | Return | `0` | - | Function Exit | - | `0` | - | 1243 | 3296 | - |

**[ NEXT: 03_RUNTIME ](lesson_runtime.html)**
