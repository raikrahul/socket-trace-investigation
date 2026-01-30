---
layout: default
title: "Era IV: Runtime"
---

# ERA IV: RUNTIME (THE EVENT)
**"And then and only then did someone call socket"**

The year is 2026. The kernel has been designed (Era I), compiled (Era II), and booted (Era III).
You type `python3` and run `socket(AF_INET, SOCK_STREAM, 0)`.

## THE SEQUENCE
Now the machine executes the path prepared for it.

### [STEP 01: THE TRAP](lessons/08_syscall_trap.md)
The CPU jumps from User Space to Kernel Space via `syscall` (MSR_LSTAR).

### [STEP 02: THE ALLOCATION](lessons/01_slab_geometry.md)
`sock_alloc()` reaches into the `sock_inode_cache` (setup in Era III) and grabs one 832-byte block.

### [STEP 03: THE LOOKUP](lessons/03_protocol_collision.md)
The kernel checks `inetsw[1]` (populated in Era III) and finds TCP.

### [STEP 04: THE ENGINE](lessons/11_russian_dolls.md)
`inet_create()` calls `sk_alloc` to create the `tcp_sock` engine (2360 bytes), attaching it to the `socket`.

### [STEP 05: THE RETURN](lessons/04_primate_arithmetic.md)
The kernel performs the `SOCKET_I` subtraction (Era II math) and returns the File Descriptor to you.

## THE END
You have now witnessed the full genesis of a socket.
