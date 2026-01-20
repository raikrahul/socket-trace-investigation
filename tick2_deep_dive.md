# TICK 2: THE DIRECTORY LOOKUP & RCU (The Array)

## THE PROBLEM (Memory Safety)
**The Naive View**:
`pf = net_families[2];` (Just read the array)

**The Reality (Concurrency)**:
What if, *at the exact nanosecond* you read index 2, the System Administrator runs `rmmod ipv4`?
1.  Admin unloads module.
2.  Kernel frees the memory at `net_families[2]`.
3.  Your CPU reads a **Dangling Pointer**.
4.  **Result**: Kernel Panic / Security Exploit.

**The Solution**: RCU (Read-Copy-Update).

---

## 1. THE CODE TRIGGER
Verified Source: `net/socket.c:1696`
```c
pf = rcu_dereference(net_families[family]);
```

## 2. THE MECHANISM (Axiomatic Breakdown)
Source: `include/linux/rcupdate.h:690`
`#define rcu_dereference(p) ... READ_ONCE(p) ...`

### Step A: READ_ONCE (Volatile Access)
The compiler is forbidden from "optimizing" this read.
It MUST generate a distinct load instruction (`MOV`) to fetch the value from RAM *right now*. It cannot cache it in a register from 10 lines ago.

### Step B: The Barrier (Ordering)
On some architectures (like ARM or PowerPC), the CPU can reorder memory reads.
`rcu_dereference` inserts a **Memory Barrier**.
**Axiom**: It ensures that you see the *content* of the struct only *after* you have successfully read the *pointer*.
(You can't read the book before you find the library).

---

## 3. THE LOOKUP CALCULATION (Real Data)

**The Array**: `net_families`
Address: `0xffffffff954767c0` (Verified on your machine).

**The Calculation**:
1.  **Input**: Family 2.
2.  **Element Size**: 8 bytes (Pointer).
3.  **Math**: `0xffffffff954767c0 + (2 * 8) = 0xffffffff954767d0`.

**The Read**:
CPU reads 8 bytes from `0xffffffff954767d0`.
Value: `0xffffffff94183a20` (The address of `inet_family_ops`).

---

## 4. CONTEXT FLOW

### THE CALLER
`__sock_create` (inside `sock_create`).
We hold the empty `sock` from Tick 1. Now we are finding out "Who knows how to fill this?"

### THE EXIT (Next Tick)
We have the pointer `pf` (Protocol Family).
We check if it's NULL (logic in `net/socket.c`).
If valid, we call `pf->create()` (Tick 3).

---

## 5. USER ERROR REPORT (BRUTAL)

**MY MISTAKES:**
1.  **IGNORED CONCURRENCY**: I treated `net_families[2]` as a static array lookup.
    *   **REALITY**: It is a concurrent data structure protected by RCU. The pointer can change at runtime.
    *   **CORRECTION**: The kernel must use `rcu_read_lock()` (implicit or explicit) and `rcu_dereference` to safely read it without locking the whole system.

2.  **SIMPLIFIED POINTER MATH**: I just said "Index * 8".
    *   **REALITY**: While true, the *compiler* turns array syntax into this math. The CPU only sees addresses.

## 6. DIAGRAM: ARRAY LOOKUP

```text
RAM ADDRESS         | CONTENT (Pointer)       | MEANING
--------------------|-------------------------|----------------
...67c0 (Index 0)   | 0x... (Unix Socket)     | family 1
...67c8 (Index 1)   | NULL (Unused)           |
...67d0 (Index 2)   | 0xffffffff94183a20      | family 2 (IPv4)  <-- READ THIS
...67d8 (Index 3)   | 0x... (Ax25)            | family 3
```
