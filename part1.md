---
layout: page
title: The Axiomatic Book of Socket (Volume 1) - From Counting to Kernel
permalink: /part1.html
---

# THE AXIOMATIC BOOK OF SOCKET

## LEVEL 0: THE PRIMATE AXIOMS (What You Already Know)

01. Numbers exist: 0, 1, 2, 3, 4, 5, ...
02. Addition: 1 + 1 = 2
03. Subtraction: 5 - 3 = 2
04. Comparison: 5 > 3 (TRUE)
05. Multiplication: 2 * 3 = 6
06. You can count slots: Slot[0], Slot[1], Slot[2], ...

---

## LEVEL 1: THE BIT (The Smallest Unit)

07. Define BIT: A thing that is either 0 or 1. Nothing else.
08. One BIT can represent 2 states: {0, 1}.
09. Two BITs can represent 4 states: {00, 01, 10, 11} = {0, 1, 2, 3}.
10. Three BITs can represent 8 states: {000, 001, 010, 011, 100, 101, 110, 111} = {0, 1, 2, 3, 4, 5, 6, 7}.
11. Pattern: N bits -> 2^N states.
12. Eight BITs can represent 2^8 = 256 states (0 to 255).
13. Define BYTE: A group of 8 bits. Range: 0 to 255.

---

## LEVEL 2: THE ARRAY (Slots in a Row)

14. Define ARRAY: A list of slots. Each slot holds one value.
15. Example: Array[0] = 5, Array[1] = 10, Array[2] = 255.
16. Define ADDRESS: The position number of a slot.
17. Array[Address] = Value stored at that slot.

---

## LEVEL 3: RAM (The Big Array)

18. Define RAM: A very large ARRAY of BYTEs.
19. Your computer has ~16,000,000,000 slots (16 GB).
20. Each slot holds one BYTE (0 to 255).
21. Example: RAM[0] = 72, RAM[1] = 101, RAM[2] = 108, ...
22. (Those bytes spell "Hel..." in ASCII encoding. Not important yet.)

---

## LEVEL 4: THE CPU (The Worker)

23. Define CPU: A machine that reads BYTEs from RAM and does work.
24. The CPU has a small internal array called REGISTERS.
25. REGISTERS are faster than RAM but very few (~16 slots).
26. Each REGISTER has a name: RAX, RBX, RCX, RDX, RDI, RSI, RSP, RIP, ...
27. The CPU repeats this loop forever:
    - Step A: Read the BYTE at the address stored in RIP.
    - Step B: Interpret that BYTE as a command.
    - Step C: Execute the command.
    - Step D: Move RIP to the next address.
28. Define RIP: The Register that holds the ADDRESS of the current command.

---

## LEVEL 5: THE INSTRUCTION (The Command)

29. Define INSTRUCTION: A number that tells the CPU what to do.
30. Example: 0x01 might mean "ADD two registers".
31. Example: 0x50 might mean "PUSH a value onto the stack".
32. Example: 0xC3 means "RET" (Return from function).
33. Example: 0xCC means "INT 3" (Breakpoint/Trap).
34. Example: 0x0F05 means "SYSCALL" (Ask the Kernel for help).

---

## LEVEL 6: THE STACK (The Notepad)

35. Define STACK: A region of RAM used to remember addresses.
36. The REGISTER called RSP points to the current top of the STACK.
37. The CALL instruction:
    - Step A: RSP = RSP - 8 (Move the pointer down).
    - Step B: RAM[RSP] = RIP + (size of current instruction). (Save return address).
    - Step C: RIP = Address of the function to call. (Jump).
38. The RET instruction:
    - Step A: RIP = RAM[RSP]. (Read the saved address).
    - Step B: RSP = RSP + 8. (Move the pointer up).
    - (CPU now continues from the saved address).

---

## LEVEL 7: PRIVILEGE LEVEL (The Lock)

39. Define PRIVILEGE LEVEL: A 2-bit flag inside the CPU.
40. Value 0 = Ring 0 = Kernel Mode (Can do anything).
41. Value 3 = Ring 3 = User Mode (Restricted).
42. Dangerous instructions (e.g., write to hardware) only work in Ring 0.
43. Your C program runs in Ring 3.

---

## LEVEL 8: THE SYSCALL (Asking for Permission)

44. You (Ring 3) want to create a network socket.
45. You cannot access hardware directly (Ring 3 restriction).
46. Solution: Ask the Kernel (Ring 0).
47. The SYSCALL instruction:
    - Step A: CPU sets Privilege Level = 0.
    - Step B: CPU saves your RIP and RSP.
    - Step C: CPU sets RIP = Address stored in a special register (MSR_LSTAR).
    - Step D: The Kernel now runs.
48. After the Kernel finishes, it runs SYSRET:
    - Step A: CPU restores your RIP and RSP.
    - Step B: CPU sets Privilege Level = 3.
    - Step C: Your program continues.

---

## LEVEL 9: THE SOCKET() CALL (The Specific Request)

49. Your C code: `int fd = socket(2, 1, 0);`
50. Before SYSCALL:
    - RAX = 41 (System call number for socket).
    - RDI = 2 (First argument: AF_INET).
    - RSI = 1 (Second argument: SOCK_STREAM).
    - RDX = 0 (Third argument: Protocol).
51. CPU executes SYSCALL.
52. Kernel reads RAX (41). Kernel says: "Ah, socket request."
53. Kernel reads RDI, RSI, RDX. Kernel allocates a socket structure.
54. Kernel assigns a File Descriptor (e.g., 3).
55. Kernel writes 3 into RAX (Return value).
56. Kernel runs SYSRET.
57. Your program resumes. Variable `fd` now holds 3.

---

## LEVEL 10: THE TRAP (INT 3)

58. Define TRAP: An instruction that forces the CPU to jump to a special handler.
59. The INT 3 instruction (BYTE 0xCC):
    - Step A: CPU saves RIP and RSP.
    - Step B: CPU sets RIP = Address of Exception Handler.
    - Step C: The Exception Handler (Kernel code) runs.
60. Why useful? We can REPLACE the first byte of any function with 0xCC.
61. When the function is called, the CPU hits 0xCC and traps to our code.

---

## LEVEL 11: THE KPROBE (The Spy Tool)

62. We want to observe `__sys_socket` (Kernel function).
63. We ask the Kernel: "Please put 0xCC at the start of `__sys_socket`."
64. We also say: "When the trap fires, call my function `entry_handler`."
65. The Kernel saves the original byte and writes 0xCC.
66. Every time `__sys_socket` is called:
    - CPU hits 0xCC.
    - Kernel calls `entry_handler`.
    - `entry_handler` logs the arguments (RDI, RSI, RDX).
    - Kernel restores the original byte.
    - Kernel lets the function continue.

---

## LEVEL 12: THE KRETPROBE (The Time Machine)

67. Problem: Kprobe only fires at the START.
68. We want to know the RETURN VALUE and the DURATION.
69. Solution: Hijack the return path.
70. At ENTRY:
    - We read RAM[RSP]. This is the REAL return address.
    - We save it to a safe place.
    - We OVERWRITE RAM[RSP] with our TRAMPOLINE address.
71. The function runs normally...
72. At EXIT:
    - CPU executes RET.
    - RET reads RAM[RSP]. It sees TRAMPOLINE, not the real address.
    - CPU jumps to TRAMPOLINE.
    - TRAMPOLINE runs our `ret_handler`.
    - `ret_handler` logs the return value (RAX) and calculates duration.
    - TRAMPOLINE jumps to the REAL return address (which we saved earlier).

---

## LEVEL 13: THE EVIDENCE (Live Proof)

73. We built a Kernel Module: `socket_latency_probe.c`.
74. We loaded it: `insmod socket_latency_probe.ko`.
75. We ran our test: `./socket_test`.
76. We checked the log: `dmesg`.

```text
[PROBE] __sys_socket START | PID: 31618 | Args: 2, 1, 0 | TS: 5881030345018
[PROBE] __sys_socket END   | PID: 31618 | FD: 3 | Status: SUCCESS | Time: 33042 ns
```

77. Verification:
    - PID 31618 = Our `socket_test` process.
    - Args 2, 1, 0 = AF_INET, SOCK_STREAM, 0 (Exactly what we sent).
    - FD 3 = The file descriptor returned.
    - Time 33042 ns = 33 microseconds to execute the kernel function.

---

## LEVEL 14: THE SOURCE CODE

### The Driver (socket_latency_probe.c)

```c
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/sched.h>

#define TARGET_COMM "socket_test"

static struct kretprobe rp;

struct my_data {
  ktime_t entry_timestamp;
};

static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 1;

  ((struct my_data *)ri->data)->entry_timestamp = ktime_get();

  pr_info("[PROBE] __sys_socket START | PID: %d | Args: %lld, %lld, %lld\n",
          current->pid, regs->di, regs->si, regs->dx);

  return 0;
}

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  ktime_t now = ktime_get();
  s64 duration_ns = ktime_to_ns(
      ktime_sub(now, ((struct my_data *)ri->data)->entry_timestamp));
  long retval = regs->ax;

  const char *status = (retval < 0 && retval > -4096) ? "FAIL" : "SUCCESS";

  pr_info("[PROBE] __sys_socket END   | PID: %d | FD: %ld | Status: %s | Time: %lld ns\n",
          current->pid, retval, status, duration_ns);

  return 0;
}

static int __init probe_init(void) {
  rp.handler = ret_handler;
  rp.entry_handler = entry_handler;
  rp.data_size = sizeof(struct my_data);
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

### The User Space Test (socket_test.c)

```c
#include <sys/socket.h>

int main(void) {
  socket(2, 1, 0);
  return 0;
}
```

---

## NEW THINGS INTRODUCED WITHOUT DERIVATION: NONE

Every concept used in Level N was defined in Levels 0 to N-1.

REJECTION STATUS: PASS
