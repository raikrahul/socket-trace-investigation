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

