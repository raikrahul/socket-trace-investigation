---
layout: page
title: Part 2 - The Proof (Live Kernel Evidence)
permalink: /part2.html
---

# PART 2: THE PROOF

## What This Document Proves

When you write `socket(AF_INET, SOCK_STREAM, 0)`, the kernel creates 3 structures and links them with pointers. This document proves it with LIVE kernel data.

---

# LEVEL 0: AXIOMS (What We Start With)

```
01. INTEGER = a number: 0, 1, 2, 3, ...
02. BYTE = 8 bits = stores 0-255
03. ADDRESS = integer that identifies a memory location
04. POINTER = address stored in memory
05. RAM = array of bytes, indexed by address
06. STRUCT = group of bytes with named fields
07. FIELD = named offset within a struct
08. LABEL = human name for a number or address
```

---

# LEVEL 1: THE PROBLEM

```
09. You write: int fd = socket(AF_INET, SOCK_STREAM, 0);
10. fd receives INTEGER 3
11. Question: What did the kernel CREATE?
12. Question: Where does fd=3 POINT to?
13. Question: How are things CONNECTED?
```

---

# LEVEL 2: THE TOOL (Kernel Probe)

To see inside the kernel, we inject a probe that prints addresses.

## The Probe Code (full_chain_probe.c)

```c
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/net.h>
#include <net/sock.h>

#define TARGET_COMM "socket_test"

static struct kretprobe rp_socket;
static struct kretprobe rp_sock_alloc;

static int socket_entry(struct kretprobe_instance *ri, struct pt_regs *regs) {
    if (strcmp(current->comm, TARGET_COMM) != 0)
        return 1;
    
    pr_info("=== SOCKET CHAIN START ===\n");
    pr_info("[1] __sys_socket ENTRY\n");
    pr_info("    PID: %d\n", current->pid);
    pr_info("    Args: family=%lu type=%lu protocol=%lu\n", 
            regs->di, regs->si, regs->dx);
    pr_info("    current = %px\n", current);
    pr_info("    current->files = %px\n", current->files);
    return 0;
}

static int socket_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
    long fd = regs->ax;
    struct file *file;
    struct socket *sock;
    struct sock *sk;
    struct fdtable *fdt;
    
    pr_info("[6] __sys_socket RETURN\n");
    pr_info("    fd = %ld\n", fd);
    
    if (fd >= 0 && fd < 1024) {
        rcu_read_lock();
        fdt = files_fdtable(current->files);
        file = fdt->fd[fd];
        rcu_read_unlock();
        
        if (file) {
            pr_info("    fdt->fd[%ld] = %px\n", fd, file);
            pr_info("    file->f_op = %px\n", file->f_op);
            pr_info("    file->private_data = %px\n", file->private_data);
            
            sock = (struct socket *)file->private_data;
            if (sock) {
                pr_info("    socket = %px\n", sock);
                pr_info("    socket->state = %d\n", sock->state);
                pr_info("    socket->type = %d\n", sock->type);
                pr_info("    socket->sk = %px\n", sock->sk);
                pr_info("    socket->ops = %px\n", sock->ops);
                pr_info("    socket->file = %px\n", sock->file);
                
                sk = sock->sk;
                if (sk) {
                    pr_info("    sock = %px\n", sk);
                    pr_info("    sock->sk_family = %d\n", sk->sk_family);
                    pr_info("    sock->sk_protocol = %d\n", sk->sk_protocol);
                    pr_info("    sock->sk_state = %d\n", sk->sk_state);
                    pr_info("    sock->sk_socket = %px\n", sk->sk_socket);
                    pr_info("    sock->sk_prot = %px\n", sk->sk_prot);
                    
                    if (sk->sk_socket == sock) {
                        pr_info("    BIDIRECTIONAL LINK VERIFIED\n");
                    }
                }
            }
        }
    }
    
    pr_info("=== SOCKET CHAIN COMPLETE ===\n");
    return 0;
}

static int sock_alloc_entry(struct kretprobe_instance *ri, struct pt_regs *regs) {
    if (strcmp(current->comm, TARGET_COMM) != 0)
        return 1;
    pr_info("[2] sock_alloc ENTRY\n");
    return 0;
}

static int sock_alloc_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
    struct socket *sock = (struct socket *)regs->ax;
    pr_info("[3] sock_alloc RETURN\n");
    pr_info("    socket = %px\n", sock);
    if (sock) {
        pr_info("    socket->state = %d\n", sock->state);
        pr_info("    socket->sk = %px (should be NULL)\n", sock->sk);
    }
    return 0;
}

static int __init probe_init(void) {
    int ret;
    
    rp_socket.handler = socket_ret;
    rp_socket.entry_handler = socket_entry;
    rp_socket.kp.symbol_name = "__sys_socket";
    ret = register_kretprobe(&rp_socket);
    if (ret < 0) {
        pr_err("Failed to register __sys_socket probe\n");
        return ret;
    }
    
    rp_sock_alloc.handler = sock_alloc_ret;
    rp_sock_alloc.entry_handler = sock_alloc_entry;
    rp_sock_alloc.kp.symbol_name = "sock_alloc";
    ret = register_kretprobe(&rp_sock_alloc);
    if (ret < 0) {
        pr_err("Failed to register sock_alloc probe\n");
        unregister_kretprobe(&rp_socket);
        return ret;
    }
    
    pr_info("[PROBE] Full Chain Probe LOADED\n");
    return 0;
}

static void __exit probe_exit(void) {
    unregister_kretprobe(&rp_socket);
    unregister_kretprobe(&rp_sock_alloc);
    pr_info("[PROBE] Full Chain Probe UNLOADED\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
```

---

# LEVEL 3: THE TEST PROGRAM (socket_test.c)

```c
#include <sys/socket.h>

int main(void) {
    socket(2, 1, 0);
    return 0;
}
```

---

# LEVEL 4: THE BUILD (Makefile)

```makefile
obj-m += full_chain_probe.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

---

# LEVEL 5: THE COMMANDS

```bash
# Step 1: Build the probe
make

# Step 2: Build the test program
gcc -o socket_test socket_test.c

# Step 3: Clear kernel log
sudo dmesg -C

# Step 4: Load the probe
sudo insmod full_chain_probe.ko

# Step 5: Run the test
./socket_test

# Step 6: Read the kernel log
sudo dmesg

# Step 7: Unload the probe
sudo rmmod full_chain_probe
```

---

# LEVEL 6: THE RAW OUTPUT

```
[13919.729335] [PROBE] Full Chain Probe LOADED
[13919.734317] === SOCKET CHAIN START ===
[13919.734321] [1] __sys_socket ENTRY
[13919.734323]     PID: 78726
[13919.734325]     Args: family=2 type=1 protocol=0
[13919.734328]     current = ffff8f4dd168d400
[13919.734330]     current->files = ffff8f4da9aba940
[13919.734333] [2] sock_alloc ENTRY
[13919.734336] [3] sock_alloc RETURN
[13919.734338]     socket = ffff8f4e33230340
[13919.734340]     socket->state = 1
[13919.734341]     socket->sk = 0000000000000000 (should be NULL)
[13919.734351] [6] __sys_socket RETURN
[13919.734352]     fd = 3
[13919.734354]     fdt->fd[3] = ffff8f4e6b37b0c0
[13919.734356]     file->f_op = ffffffff9416e9a0
[13919.734357]     file->private_data = ffff8f4e33230340
[13919.734359]     socket = ffff8f4e33230340
[13919.734361]     socket->state = 1
[13919.734362]     socket->type = 1
[13919.734364]     socket->sk = ffff8f4e2e47e880
[13919.734366]     socket->ops = ffffffff94183c00
[13919.734367]     socket->file = ffff8f4e6b37b0c0
[13919.734369]     sock = ffff8f4e2e47e880
[13919.734371]     sock->sk_family = 2
[13919.734372]     sock->sk_protocol = 6
[13919.734374]     sock->sk_state = 7
[13919.734375]     sock->sk_socket = ffff8f4e33230340
[13919.734377]     sock->sk_prot = ffffffff953e2400
[13919.734379]     BIDIRECTIONAL LINK VERIFIED
[13919.734380] === SOCKET CHAIN COMPLETE ===
```

---

# LEVEL 7: PARSING THE OUTPUT (Line by Line)

## Line 01: Process Identity
```
PID: 78726
```
- 78726 = INTEGER
- This is the process ID of socket_test

## Line 02: Arguments
```
Args: family=2 type=1 protocol=0
```
- 2 = INTEGER (AF_INET = IPv4)
- 1 = INTEGER (SOCK_STREAM = TCP)
- 0 = INTEGER (default protocol)

## Line 03: Task Struct
```
current = ffff8f4dd168d400
```
- ffff8f4dd168d400 = POINTER (64-bit address)
- Points to: struct task_struct (the process)

## Line 04: Files Struct
```
current->files = ffff8f4da9aba940
```
- files = FIELD name at offset in task_struct
- ffff8f4da9aba940 = POINTER
- Points to: struct files_struct (file descriptors)

## Line 05: Socket Allocation (Before Linking)
```
socket = ffff8f4e33230340
socket->state = 1
socket->sk = 0000000000000000
```
- ffff8f4e33230340 = POINTER (new struct socket)
- state = 1 = INTEGER (SS_UNCONNECTED)
- sk = 0 = NULL POINTER (not linked yet)

## Line 06: Return Value
```
fd = 3
```
- 3 = INTEGER (file descriptor number)
- This is what user space receives

## Line 07: File Descriptor Table Entry
```
fdt->fd[3] = ffff8f4e6b37b0c0
```
- fd[3] = ARRAY slot at index 3
- ffff8f4e6b37b0c0 = POINTER
- Points to: struct file

## Line 08: File Structure
```
file->f_op = ffffffff9416e9a0
file->private_data = ffff8f4e33230340
```
- f_op = POINTER to socket_file_ops (function table)
- private_data = POINTER to struct socket

## Line 09: Socket Structure (After Linking)
```
socket = ffff8f4e33230340
socket->state = 1
socket->type = 1
socket->sk = ffff8f4e2e47e880
socket->ops = ffffffff94183c00
socket->file = ffff8f4e6b37b0c0
```
- socket = POINTER (same address as before)
- sk = POINTER (now points to struct sock!)
- ops = POINTER to inet_stream_ops
- file = POINTER (back to struct file)

## Line 10: Sock Structure
```
sock = ffff8f4e2e47e880
sock->sk_family = 2
sock->sk_protocol = 6
sock->sk_state = 7
sock->sk_socket = ffff8f4e33230340
sock->sk_prot = ffffffff953e2400
```
- sock = POINTER (struct sock address)
- sk_family = 2 = INTEGER (AF_INET)
- sk_protocol = 6 = INTEGER (IPPROTO_TCP)
- sk_state = 7 = INTEGER (TCP_CLOSE)
- sk_socket = POINTER (back to struct socket!)
- sk_prot = POINTER to tcp_prot

---

# LEVEL 8: THE CHAIN DIAGRAM

## Step-by-Step Construction

```
BEFORE socket() call:
    fd = ??? (undefined)
    No kernel structures exist for this socket

AFTER socket() call:
    fd = 3 (integer returned to user space)
    3 kernel structures created and linked
```

## The Complete Chain

```
USER SPACE:
    int fd = 3

        |
        | (syscall boundary)
        v

KERNEL SPACE:

    current (task_struct) @ 0xffff8f4dd168d400
        |
        +---> files (files_struct) @ 0xffff8f4da9aba940
                |
                +---> fdt->fd[3] = 0xffff8f4e6b37b0c0
                                    |
                                    v
                    +=======================================+
                    | struct file @ 0xffff8f4e6b37b0c0      |
                    |---------------------------------------|
                    | f_op          = 0xffffffff9416e9a0    |
                    | private_data  = 0xffff8f4e33230340 ---|--+
                    +=======================================+  |
                                                               |
                              +--------------------------------+
                              |
                              v
                    +=======================================+
                    | struct socket @ 0xffff8f4e33230340    |
                    |---------------------------------------|
                    | state  = 1 (SS_UNCONNECTED)           |
                    | type   = 1 (SOCK_STREAM)              |
                    | file   = 0xffff8f4e6b37b0c0 (back) ---|---> file
                    | ops    = 0xffffffff94183c00           |
                    | sk     = 0xffff8f4e2e47e880 ----------|--+
                    +=======================================+  |
                              ^                                |
                              |                                |
                              +--------------------------------+
                              |                                |
                              v                                v
                    +=======================================+
                    | struct sock @ 0xffff8f4e2e47e880      |
                    |---------------------------------------|
                    | sk_family    = 2 (AF_INET)            |
                    | sk_protocol  = 6 (TCP)                |
                    | sk_state     = 7 (TCP_CLOSE)          |
                    | sk_socket    = 0xffff8f4e33230340 ----|---> socket
                    | sk_prot      = 0xffffffff953e2400     |
                    +=======================================+
```

---

# LEVEL 9: PROOF OF BIDIRECTIONAL LINKING

## Forward Links (Top to Bottom)
```
fd[3] -----> file -----> socket -----> sock
                (private_data)    (sk)
```

## Backward Links (Bottom to Top)
```
sock -----> socket -----> file
  (sk_socket)     (file)
```

## Address Verification
```
socket->sk       = 0xffff8f4e2e47e880  (points to sock)
sock->sk_socket  = 0xffff8f4e33230340  (points to socket)

socket address   = 0xffff8f4e33230340
sock address     = 0xffff8f4e2e47e880

VERIFIED: They point to each other!
```

---

# LEVEL 10: WHAT IS EACH STRUCTURE?

## struct file (VFS Layer)
```
Purpose: Universal file handle
Created by: sock_alloc_file()
Contains:
    - f_op: Function pointers (read, write, poll)
    - private_data: Pointer to actual object (socket)
```

## struct socket (Socket Layer)
```
Purpose: Protocol-independent socket abstraction
Created by: sock_alloc()
Contains:
    - state: Connection state (UNCONNECTED, CONNECTED)
    - type: Socket type (STREAM, DGRAM)
    - sk: Pointer to transport layer (sock)
    - file: Pointer back to file
    - ops: Protocol operations (inet_stream_ops)
```

## struct sock (Transport Layer)
```
Purpose: TCP/UDP protocol state
Created by: sk_alloc()
Contains:
    - sk_family: Address family (AF_INET)
    - sk_protocol: Protocol number (6 = TCP)
    - sk_state: TCP state (CLOSE, LISTEN, ESTABLISHED)
    - sk_socket: Pointer back to socket
    - sk_prot: Protocol operations (tcp_prot)
```

---

# LEVEL 11: THE COMPLETE DATA

## All Integers
```
fd = 3
family = 2
type = 1
protocol = 0
sk_family = 2
sk_protocol = 6
sk_state = 7
socket->state = 1
socket->type = 1
PID = 78726
```

## All Pointers
```
current          = 0xffff8f4dd168d400
current->files   = 0xffff8f4da9aba940
fdt->fd[3]       = 0xffff8f4e6b37b0c0
file->f_op       = 0xffffffff9416e9a0
file->private    = 0xffff8f4e33230340
socket->sk       = 0xffff8f4e2e47e880
socket->file     = 0xffff8f4e6b37b0c0
socket->ops      = 0xffffffff94183c00
sock->sk_socket  = 0xffff8f4e33230340
sock->sk_prot    = 0xffffffff953e2400
```

## All Labels
```
current, files, fdt, fd, f_op, private_data
socket, state, type, sk, ops, file
sock, sk_family, sk_protocol, sk_state, sk_socket, sk_prot
```

---

# LEVEL 12: FINAL PROOF

## The socket() syscall creates:

```
3 STRUCTURES:
    struct sock   @ 0xffff8f4e2e47e880
    struct socket @ 0xffff8f4e33230340
    struct file   @ 0xffff8f4e6b37b0c0

6 LINKING POINTERS:
    fd[3] -> file
    file->private_data -> socket
    socket->sk -> sock
    socket->file -> file
    sock->sk_socket -> socket
    
1 RETURN VALUE:
    fd = 3 (integer)
```

## Conclusion

The entire networking stack is:
- INTEGERS (numbers with meaning)
- POINTERS (addresses linking structures)
- LABELS (field names for human readability)

**Nothing else. No magic. Just data and connections.**

---

**END OF PROOF**
