# AXIOMATIC SOCKET CREATION: TICK 0 & TICK 1

01. Symbol = A name for an Address.
02. Compiler = Maps Code Names to Symbols.
03. Linker = Maps Symbols to Final Memory Addresses.
04. Kernel Image = A file containing Symbols and Addresses.
05. Boot = Loading Kernel Image into RAM.
06. RAM = Array of bytes.
07. RAM[Address] = Data.

## TICK 0: COMPILER AND LINKER SETUP

08. C-Code: `struct kmem_cache *sock_inode_cachep;` (net/socket.c:302)
09. 08 → Symbol `sock_inode_cachep`.
10. Compiler sees 09 → reserve 8 bytes in Data Section.
11. Linker sees 10 → assign Address (e.g., 0xffffffff826a4b10).
12. Linker → `sock_inode_cachep` now refers to Address 0xffffffff826a4b10.
13. Value at 12 = 0 (Initialized to zero by default).

14. C-Code: `struct socket_alloc { ... }` (include/net/sock.h:1555)
15. 14 -> size calculation by Compiler.
16. sizeof(struct socket) = 128 (Derived in Tick 1).
17. sizeof(struct inode) = 592 (System dependent).
18. THEREFORE sizeof(struct socket_alloc) = 128 + 592 = 720.

## TICK 0: BOOT TIME (INITIALIZATION)

19. Boot Loader -> Copy Data Section to RAM.
20. RAM[0xffffffff826a4b10] = 0.
21. Kernel calls `sock_init()`.
22. `sock_init()` calls `kmem_cache_create("sock_inode_cache", 720, ...)`.
23. 22 -> Kernel allocates memory pool for 720-byte blocks.
24. 23 -> returns Address of pool controller (e.g., 0xffff8f4e00001000).
25. RAM[0xffffffff826a4b10] = 0xffff8f4e00001000.
26. THEREFORE `sock_inode_cachep` now points to the socket memory pool. [OK]

## TICK 1: PHASE 1 (SOCKET ALLOCATION)

27. User calls `socket()`.
28. Kernel calls `sock_alloc()`.
29. `sock_alloc()` calls `sock_alloc_inode()`.
30. `sock_alloc_inode()` derives base address from `sock_inode_cachep`.
31. Base Address = RAM[0xffffffff826a4b10] = 0xffff8f4e00001000.
32. `alloc_inode_sb(sb, Base Address, GFP_KERNEL)` starts.
33. 32 -> Find a free 720-byte slot in the pool.
34. 33 -> Slot Address = 0xffff8f4e33230340.
35. `ei = 0xffff8f4e33230340`.
36. 35 -> Address of the new `struct socket_alloc`.
37. offsetof(socket_alloc, socket) = 0.
38. 36 + 37 -> Address of `struct socket` = 0xffff8f4e33230340.
39. offsetof(socket_alloc, vfs_inode) = 128.
40. 36 + 39 -> Address of `struct inode` = 0xffff8f4e332303c0.
41. `sock_alloc_inode` returns 0xffff8f4e332303c0. [OK]

---

NEW THINGS INTRODUCED WITHOUT DERIVATION:
- Data Section (Memory region for global variables)
- Boot Loader (Software that starts the kernel)
- kmem_cache (Kernel structure for memory pools)
- GFP_KERNEL (Flag for memory allocation type)
