---
layout: page
title: The Axiomatic Book of Socket (Volume 1)
permalink: /part1.html
---

# The Axiomatic Book of Socket: Volume 1

## Chapter 1: The Request (User Space)

**01. The Axiom of the User**
A user program is a sequence of instructions executing in a restricted mode (Ring 3). It cannot touch hardware. It cannot allocate physical RAM. It cannot send network packets. To do these things, it must ask the Kernel.

**02. The Function Call**
The user writes:
```c
int fd = socket(AF_INET, SOCK_STREAM, 0);
```
To the user, this is a single line.
To the CPU, this is a sequence of preparations:
1.  **Load Argument 1 (Family)**: `AF_INET (2)` -> Register `RDI`.
2.  **Load Argument 2 (Type)**: `SOCK_STREAM (1)` -> Register `RSI`.
3.  **Load Argument 3 (Protocol)**: `0` -> Register `RDX`.
4.  **Load System Call Number**: `41` (socket) -> Register `RAX`.

**03. The Barrier (SYSCALL)**
The user code executes the `syscall` instruction.
*   **Axiom**: `syscall` causes a privilege level transition.
*   **Effect**: The CPU jumps to a specific address in the Kernel (Ring 0), defined in the `MSR_LSTAR` register.
*   **State Change**: User Space execution pauses. Kernel Space execution begins.

---

## Chapter 2: The Kernel Entry (__sys_socket)

**04. The Destination**
The kernel's entry point eventually calls `__sys_socket`.
*   **Symbol**: `__sys_socket`
*   **Address**: `0xffffffff81...` (Obscured by KASLR).
*   **Purpose**: To interpret the user's request and create a generic socket structure.

**05. The Problem of Observation**
We want to know:
1.  Did the arguments arrive correctly?
2.  How long did it take?
3.  What was the result?
*   **Constraint**: We cannot modify the kernel source code and recompile Linux. We must observe *live*.

---

## Chapter 3: The Instrument (Kretprobe)

**06. The Concept of the Trap**
To observe `__sys_socket` without recompiling, we typically use a "Trap".
*   **Action**: We overwrite the *first byte* of `__sys_socket` with `0xCC` (INT 3).
*   **Effect**: When the CPU tries to run `__sys_socket`, it hits `0xCC` and triggers an exception.
*   **Handler**: The Kernel pauses, calls our code, and then resumes the original function.

**07. The Mechanism of Time Travel (Return Probing)**
We need to measure *Duration* (Start to End).
*   **Problem**: A standard Trap only stops at the *Start*.
*   **Solution**: Kretprobe (Kernel Return Probe).
    *   **Step 1**: Trap at Start.
    *   **Step 2**: Record "Start Time".
    *   **Step 3 (The Hijack)**: We must force the CPU to wake us up at the *End*.
    *   **Execution**: We find the "Return Address" on the Stack (RAM). We **overwrite** it with the address of our "Trampoline".

**08. The Axiom of the Stack Hijack**
*   **Before**: Stack Top = `0xReallReturnAddress`
*   **After**: Stack Top = `0xTrampolineAddress`
*   **Why**: When `__sys_socket` finishes and runs `RET`, it pops the Value from the Stack. It pops our Trampoline. It jumps to US.

---

## Chapter 4: The Evidence (Forensics)

**09. The Experiment**
We wrote a Kernel Module (`socket_latency_probe.c`) that:
1.  Registers a Kretprobe on `__sys_socket`.
2.  **Entry Handler**:
    *   Checks PID (ignore ping, ignore shell).
    *   Records `ktime_get()`.
3.  **Exit Handler**:
    *   Calculates `Now - Start`.
    *   Reads `Regs->AX` (The Return Value).
    *   Logs the truth.

**10. The Result**
We observed the following atomic events in the Kernel Log (`dmesg`):

```text
[ 5881.101484] [PROBE] __sys_socket START | PID: 31618 | Args: 2, 1, 0 | TS: 5881030345018
[ 5881.101516] [PROBE] __sys_socket END   | PID: 31618 | FD: 3 | Status: SUCCESS | Time: 33042 ns
```

**11. The Interpretation**
1.  **PID Match**: 31618 matches our user-space tool.
2.  **Args Confirmed**: `2` (AF_INET), `1` (SOCK_STREAM). The kernel received exactly what we sent.
3.  **Success**: FD `3` was returned.
4.  **Cost**: The operation took **33,042 nanoseconds** (33 microseconds).

---

## Chapter 5: The Source Code

**12. The Driver (socket_latency_probe.c)**
This code implements the Axioms of Chapter 3.
(See attached artifact in Section 7 of the full report).

**13. The User Space (socket_test.c)**
This code implements the Axioms of Chapter 1.
(See attached artifact).

**14. The Build (Makefile)**
This code instructs the compiler how to build Chapter 5.
(See attached artifact).



---


# Appendix A: Raw Forensic Logs
# **FORENSIC KERNEL TRACE REPORT: socket(AF_INET, SOCK_STREAM, 0)**

---

## **PHASE 1: LIVE PROBE VERIFICATION (kprobes)**

01. **🛠️ PROBE_INIT** → `insmod socket_probe.ko` .......... [Driver Activated] ✓
02. **👤 USER_EXEC** → `./socket_test` .......... [Trigger Syscall 41] ✓
03. **⚙️ STEP_09: CHASSIS_CREATED** ← `sock_alloc()` .......... [[PROBE] sock_alloc Exit: socket=f1166dcd] ✓
04. **🔍 STEP_10: PRE_LINK_CHECK** → `sk == NULL` .......... [[PROBE] Chassis Init: sk=0, ops=0, state=1] ✓
05. **🔗 STEP_13: LINK_ESTABLISHED** ← `inet_create()` .......... [[PROBE] inet_create Entry] ✓
06. **📑 STEP_14: VFS_WRAP** ← `sock_alloc_file()` .......... [[PROBE] fd_install Entry: file=3335ec5d] ✓
07. **📂 STEP_15: PRIVATE_DATA_PROBE** → `file->private_data == socket` .......... [[PROBE] private_data=f1166dcd] ✓
08. **🔄 STEP_13_BACKLINK: SYMMETRY_PROBE** → `sk->sk_socket == socket` .......... [[PROBE] sk->sk_socket=f1166dcd] ✓
09. **🔢 STEP_16: INDEX_VERIFIED** → `fd == 3` .......... [[PROBE] fd_install Entry: fd=3] ✓
10. **📤 STEP_17: syscall_RETURN** → `Result == 3` .......... [[PROBE] __sys_socket Return: fd=3] ✓

---

## **PHASE 2: MEMORY LINKAGE MAP (Actual Addresses)**

```text
[VFS Layer: struct file]   @ 0x...3335ec5d
      |
      +--> private_data ---+
                           |
[Socket Layer: struct socket] <---+ @ 0x...f1166dcd
      |                    ^
      +--> sk ---------+   |
                       |   |
[Stack Layer: struct sock] <---+ @ 0x...
      |                    |
      +--> sk_socket ------+ (Back-pointer)
```

---

## **PHASE 3: THE HARDER PUZZLE (NULL DEREFERENCE PREVENTION)**

**Puzzle**: What if a signal interrupts the process *after* `sock_alloc` (Step 09) but *before* `fd_install` (Step 16)?

01. **State**: Memory allocated 🏗️, but not in FD table ✗.
02. **Threat**: User-space cannot `close()` the fd (it doesn't exist yet).
03. **Solution**: The kernel maintains a local reference `struct file *`.
04. **Trap**: If `__sys_socket` fails later (e.g. `fd_install` fails), it calls `sock_release()`.
05. **Action**: `sock_release` calls `iput(inode)` which frees `struct socket` and `struct sock`.
06. **Result**: Zero memory leak. No zombie sockets. ✓

---

### **NEW THINGS INTRODUCED WITHOUT DERIVATION:**
- kprobes (Mechanism)
- dmesg (Log sink)
- sock_release (Cleanup function)
- iput (Cleanup mechanism)

### **REJECTION STATUS: PASS**
- **Linear Trace**: Probes verified the exactly derived sequence.
- **Data Integrity**: Addresses matched across file and socket levels.
- **Symmetry**: `sk <-> socket` linkage verified via live pointer inspection.


---


# Appendix B: The Chain of Command
01. 🛠️ SYSCALL_41 (socket) → [AF_INET, SOCK_STREAM, 0] ✓
02. ⚙️ __sys_socket Entry → [family=2, type=1, protocol=0] ✓
03. 🏗️ sock_alloc Return → [socket_ptr=ffff8ae8074cde40] ✓
04. 🔗 inet_create Entry → [socket_ptr=ffff8ae8074cde40] ✓
05. ∴ ffff8ae8074cde40 == ffff8ae8074cde40 → [TRUE] ✓

---

# AXIOMATIC CHAIN PROOF

6. 📄 socket.c:1706 → __sys_socket calls __sys_socket_create
7. 📄 socket.c:1664 → __sys_socket_create calls sock_create
8. 📄 socket.c:1627 → sock_create calls __sock_create
9. 📄 socket.c:1535 → __sock_create calls sock_alloc (Address Allocation)
10. 📄 socket.c:1571 → __sock_create calls pf->create (inet_create)
11. ∴ The pointer ffff8ae8074cde40 is the "Common Thread" through the chain. ✓

---

# MEMORY MAP SYMMETRY

[__sys_socket]
      |
      +--> [sock_alloc] 
               |
               *---(ffff8ae8074cde40)---> Result ✓
                                           |
[inet_create]                              |
      |                                    |
      *<---(ffff8ae8074cde40)--------------* ✓

---

NEW THINGS INTRODUCED WITHOUT DERIVATION: None.
NEW INFERENCE: None.
REJECTION STATUS: PASS


---


# Appendix C: The Source Code
```c
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/sched.h>

#define TARGET_COMM "socket_test"

/*
 * LATENCY & ERROR PROBE
 *
 * 1. STATEFUL PROBING: We need to remember "When did we start?"
 *    Solution: kretprobe allows us to allocate 'data' (a backpack)
 *    that travels with the function call.
 *
 * 2. TIMING:
 *    Axiom: ktime_get() returns current nanoseconds.
 *    Math: End - Start = Duration.
 *
 * 3. ERROR CHECKING:
 *    Axiom: In Kernel, integers > -4095 (0xfffff000) are Error Codes.
 *    (Because valid pointers/FDs are positive/lower).
 */

static struct kretprobe rp;

// The "Backpack" structure
struct my_data {
  ktime_t entry_timestamp;
};

// RUNS AT FUNCTION ENTRY (Start)
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, "ping") == 0) {
    pr_info("[PROBE] SKIPPING PING | PID: %d | Returning 1 (No Kretprobe)\n",
            current->pid);
    return 1;
  }

  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 1; // Return 1 = Skip the Return Probe (Optimization)

  ((struct my_data *)ri->data)->entry_timestamp = ktime_get();

  // Log the Request (like before)
  pr_info("[PROBE] __sys_socket START | PID: %d | Args: %lld, %lld, %lld | TS: "
          "%lld\n",
          current->pid, regs->di, regs->si, regs->dx,
          ktime_to_ns(((struct my_data *)ri->data)->entry_timestamp));

  return 0;
}

// RUNS AT FUNCTION EXIT (End)
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  ktime_t now = ktime_get();
  s64 duration_ns = ktime_to_ns(
      ktime_sub(now, ((struct my_data *)ri->data)->entry_timestamp));
  long retval = regs->ax;

  // Error Axiom: IS_ERR_VALUE check (simplified)
  const char *status = (retval < 0 && retval > -4096) ? "FAIL" : "SUCCESS";

  pr_info("[PROBE] __sys_socket END   | PID: %d | FD: %ld | Status: %s | Time: "
          "%lld ns\n",
          current->pid, retval, status, duration_ns);

  return 0;
}

static int __init probe_init(void) {
  rp.handler = ret_handler;
  rp.entry_handler = entry_handler;
  rp.data_size = sizeof(struct my_data); /* Reserve space for backpack */
  rp.kp.symbol_name = "__sys_socket";

  if (register_kretprobe(&rp) < 0) {
    pr_err("register_kretprobe failed\n");
    return -1;
  }
  pr_info("[PROBE] Latency Probe Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kretprobe(&rp);
  pr_info("[PROBE] Latency Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
```
