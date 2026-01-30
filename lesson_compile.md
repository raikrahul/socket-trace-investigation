---
layout: default
title: "Era II: Compile Time"
---

# ERA II: COMPILE TIME (THE FROZEN TRUTH)
**"When kernel got compiled"**

The source code was fed into `gcc`. The compiler took the abstract design from Era I and froze it into immutable binary facts.

## 01. THE LINKER'S SEAL
The compiler calculated the addresses of every function and global variable. It decided *where* they would live forever (subject to boot-time sliding).

### The Inetsw Array
The compiler reserved space for the Protocol Switch Table.
```bash
$ nm vmlinux | grep inetsw
ffffffff838abc00 D inetsw  # Placed in .data section
```
*Fact*: At this moment, `inetsw` is empty. It is a bucket of zeros waiting for Boot Time.

### The Operations Table
The compiler sealed the File System operations into Read-Only memory.
```bash
$ nm vmlinux | grep sockfs_ops
ffffffff8276e740 R sockfs_ops # Placed in .rodata
```
*Fact*: `R` means Read-Only. The logic for creating sockets is now immutable.

## 02. THE PREPROCESSOR TRUTH
The preprocessor replaced human-readable macros with hard numbers.

### The Container Math
The compiler analyzed `SOCKET_I(inode)`:
```c
struct socket *SOCKET_I(struct inode *inode) {
    return &container_of(inode, struct socket_alloc, vfs_inode)->socket;
}
```
Knowing from Era I that `vfs_inode` is at offset 128, the compiler replaced this entire function with a single instruction:
```asm
lea -0x80(%rax), %r12  ; Subtract 128 (0x80) from inode address
```
*Fact*: There is no function call at runtime. Just a subtraction.

## 03. THE INITCALL LIST
The compiler gathered all functions marked `__init` (like `sock_init` and `inet_init`) and placed pointers to them in a special section `.init.text`.
This created the "To-Do List" for the boot process.

**[ NEXT: ERA III - BOOT TIME ](lesson_boot.html)**
