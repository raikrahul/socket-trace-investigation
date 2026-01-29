---
layout: page
title: The Complete Socket Investigation (Axiomatic)
permalink: /consolidated.html
---

# 1. The Forensic Evidence (Live Trace)
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


# 2. The Chain of Command (Call Chain Proof)
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


# 3. The Execution Trace (Pure ASCII)
#1.Call.SYSCALL_DEFINE3(socket).Values.family=2,type=1,proto=0.Data.AF_INET,SOCK_STREAM,0.Work.Entry from userspace via syscall vector.Errors.None.RealValue.41.RealData.regs->orig_rax.Caller.0.Current.1723.Resumed.0
#2.Call.__sys_socket.Values.2,1,0.Data.AF_INET,SOCK_STREAM,0.Work.Main kernel entry point for socket creation logic.Errors.None.RealValue.int.RealData.2,1,0.Caller.1725.Current.1706.Resumed.0
#3.Call.update_socket_protocol.Values.2,1,0.Data.AF_INET,SOCK_STREAM,0.Work.Normalizing protocol for the family.Errors.None.RealValue.0.RealData.0.Caller.1712.Current.1706.Resumed.0
#4.Call.sock_create.Values.2,1,0.Data.AF_INET,SOCK_STREAM,0.Work.Wrapper call to internal creation function.Errors.None.RealValue.int.RealData.0.Caller.1711[Inlined].Current.1625.Resumed.0
#5.Call.__sock_create.Values.net_ns,2,1,0,&sock,0.Data.NET,AF_INET,SOCK_STREAM,0,struct socket**,kern=0.Work.Core logic allocation and initialization sequence.Errors.None.RealValue.int.RealData.0.Caller.1627.Current.1500.Resumed.0
#6.Call.security_socket_create.Values.2,1,0,0.Data.AF_INET,SOCK_STREAM,0,kern=0.Work.LSM Access control check before allocation.Errors.None.RealValue.0.RealData.0.Caller.1526.Current.1526.Resumed.0
#7.Call.sock_alloc.Values.void.Data.void.Work.Memory allocation of struct socket and inode.Errors.None.RealValue.ffff8ae8...[Valid Pointer].RealData.struct socket address.Caller.1535.Current.1535.Resumed.0
#8.Call.pf->create.Values.net,sock,0,0.Data.NET,ffff8ae8...,0,0.Work.Protocol specific init (inet_create).Errors.None.RealValue.0.RealData.0.Caller.1571.Current.1571.Resumed.0
#9.Call.security_socket_post_create.Values.sock,2,1,0,0.Data.ffff8ae8...,AF_INET,SOCK_STREAM,0,0.Work.LSM Post-init check.Errors.None.RealValue.0.RealData.0.Caller.1592.Current.1592.Resumed.0
#10.Call.sock_map_fd.Values.sock,0.Data.ffff8ae8...,flags=0.Work.Install socket into process FD table and return index.Errors.None.RealValue.3.RealData.File Descriptor.Caller.1720.Current.1720.Resumed.0
#11.Return.SYSCALL_DEFINE3(socket).Values.3.Data.FD=3.Work.Syscall complete. Result returned to user regs->rax.Errors.None.RealValue.3.RealData.3.Caller.0.Current.1725.Resumed.1723


---


# 4. The Full Socket Flow (User to Kernel)
# AXIOMATIC SOCKET FLOW: USER → KERNEL
> "From `main()` to `Memory` and back again."

---

## 🛑 PRE-REQUISITE DEFINITIONS
1.  **Syscall 41**: The unique number for `socket` on x86_64.
2.  **RDI, RSI, RDX**: The CPU registers used for arguments 1, 2, and 3.
3.  **RAX**: The CPU register used for the Return Value.

---

## 🖥️ PHASE 1: USER SPACE (The Request)

01. **CODE**: `int fd = socket(AF_INET, SOCK_STREAM, 0);`
02. **CONSTANTS (Axiom)**:
    *   `AF_INET` = 2
    *   `SOCK_STREAM` = 1
    *   `Protocol` = 0
03. **ASSEMBLY (Action)**:
    *   `MOV RAX, 41` (Prepare Syscall Number)
    *   `MOV RDI, 2` (Arg 1: Family)
    *   `MOV RSI, 1` (Arg 2: Type)
    *   `MOV RDX, 0` (Arg 3: Protocol)
    *   `SYSCALL` (Trigger CPU Interrupt/Trap)
    
---

## 🚪 PHASE 2: THE SYSCALL GATE (The Transition)

04. **CPU**: Switches from Ring 3 (User) to Ring 0 (Kernel).
05. **TABLE**: Looks up `sys_call_table[41]`.
06. **TARGET**: Found `__x64_sys_socket`.
07. **JUMP**: CPU execution moves to Kernel Memory addresses.

---

## ⚙️ PHASE 3: KERNEL ENTRY (The Dispatch)

08. **FUNCTION**: `__sys_socket(2, 1, 0)`
09. **LOCATION**: `net/socket.c:1706`
10. **ACTION**: Calls `__sys_socket_create(2, 1, 0)`.
11. **OPTIMIZATION (Proof)**: Compiler Inlines `__sys_socket_create`. Stack trace shows direct jump to next logic.

---

## 🏭 PHASE 4: THE CREATE CHAIN (The Logic)

12. **FUNCTION**: `__sock_create(net, 2, 1, 0, ...)`
13. **LOCATION**: `net/socket.c:1500`
14. **CHECK**: Validates family (2) < NPROTO.
15. **CHECK**: Validates type (1) < SOCK_MAX.

---

## 🛡️ PHASE 5: SECURITY CHECK (LSM)

16. **FUNCTION**: `security_socket_create(2, 1, 0, ...)`
17. **PROBE PROOF**: `security_probe.c` logged ENTRY.
18. **WORK**: Checks AppArmor/SELinux policies.
19. **RESULT**: Allowed (0).

---

## 🏗️ PHASE 6: MEMORY ALLOCATION (The Object)

20. **FUNCTION**: `sock_alloc()`
21. **LOCATION**: `net/socket.c:1535`
22. **INPUT**: None (void).
23. **WORK**:
    *   Calls `new_inode_pseudo()` to get an inode.
    *   Allocates `struct socket` on the Heap.
24. **PROBE PROOF**: `sock_alloc_probe.c` captured Return Value `0xffff...`.
25. **RESULT**: A valid, empty Socket Object in Kernel Memory.

---

## 🔌 PHASE 7: PROTOCOL INITIALIZATION (The Setup)

26. **FUNCTION**: `inet_create(net, sock, 0)`
27. **LOCATION**: `net/ipv4/af_inet.c:191`
28. **INPUT**: The `sock` pointer from Phase 6.
29. **WORK**:
    *   Sets `sock->ops` to `inet_stream_ops`.
    *   Allocates `struct sock` (The Network Layer representation).
    *   Sets state to `SS_UNCONNECTED`.
30. **PROBE PROOF**: `inet_create_probe.c` saw the SAME `sock` address as Phase 6.
31. **LINKAGE**: We have now linked the Generic Socket to TCP/IP Logic.

---

## 📂 PHASE 8: FILE MAPPING (The Handle)

32. **FUNCTION**: `sock_map_fd(sock, flags)`
33. **LOCATION**: `net/socket.c:1720`
34. **INPUT**: The initialized `sock`.
35. **WORK**:
    *   finds an empty slot in the Process File Table.
    *   e.g., Slot 3.
    *   Links Slot 3 → `struct file` → `struct socket`.
36. **RESULT**: Integer `3`.

---

## ↩️ PHASE 9: RETURN (The Result)

37. **KERNEL**: Puts `3` into `RAX`.
38. **INSTRUCTION**: `SYSRET` (Return from Ring 0 to Ring 3).
39. **USER SPACE**: `fd = 3`.
40. **PROOF**: `./socket_test` printed "Socket Created: 3".

---

## 🔗 SUMMARY OF EVIDENCE (The Files)

| Phase | Evidence File | Method | Meaning |
| :--- | :--- | :--- | :--- |
| User | `socket_test.c` | Source | The Request Origin |
| Alloc | `sock_alloc_probe.c` | kretprobe | The Object Birth |
| Chain | `caller_probe.c` | Stack Trick | The Parentage (`__sock_create`) |
| Init | `inet_create_probe.c` | kprobe | The Protocol Bind |
| Gapless | `security_probe.c` | kprobe | The Continuity |
| Risk | `puzzle_probe.c` | Logic Check | The Leak Prevention Safety Net |

---
**Q.E.D.**


---


# 5. The Mechanism of Kretprobes (Stack Hijack)
# Axiomatic Derivation: The Mechanism of Kretprobe

## 1. The Physical Reality (Axioms)

01. **CPU Registers**: The "Hands" of the CPU. Fast, temporary storage.
    *   **RSP (Stack Pointer)**: A specific register that holds a *Number*. This number is an *Address* pointing to a location in RAM.
    *   **RIP (Instruction Pointer)**: A specific register that holds the Address of the *current* instruction attempting to execute.

02. **RAM (Random Access Memory)**: The "Notebook". Massive grid of storage bytes.
    *   **The Stack**: A reserved region in RAM.
    *   **The Text Segment**: A Read-Only region in RAM where code lives.

03. **Opcode `CALL`**: When CPU executes `CALL function`:
    1.  Calculates the Address of the *next* instruction (Result: `0xRET_ADDR`).
    2.  Writes `0xRET_ADDR` to the RAM location pointed to by `RSP`.
    3.  Decrements `RSP` (Stack Grows Down).
    4.  Updates `RIP` to the address of `function`.

04. **Opcode `RET`**: When CPU executes `RET`:
    1.  Increments `RSP`.
    2.  Reads the value from RAM at the location pointed to by `RSP`.
    3.  Updates `RIP` with that value (Jumps back).

---

## 2. The Setup (Before The Trap)

**Scenario**: User calls `socket()`.
*   User Code Address: `0x100` calling function at `0x1000`.
*   Return Address: `0x105` (Instruction after call).
*   Stack Pointer (`RSP`): `0x9000`.

**State at T=0 (Just before execution)**:
*   `RIP`: `0x100`
*   `RSP`: `0x9008`
*   RAM[`0x9000`]: [Empty/Zero]

**State at T=1 (After CALL executes)**:
*   `RIP`: `0x1000` (Start of `__sys_socket`)
*   `RSP`: `0x9000`
*   RAM[`0x9000`]: **0x105** (The Real Return Address)

---

## 3. The Trap (Context Switch)

**The Intervention**: We registered a `kretprobe` at `__sys_socket` (`0x1000`).
The Kernel has physically overwritten the byte at `0x1000` with `INT 3` (Breakpoint).

**State at T=2 (CPU hits 0x1000)**:
1.  **Exception**: CPU encounters `INT 3`.
2.  **Context Save**: CPU saves current registers (including `RSP=0x9000`) to a struct `pt_regs` in Kernel Memory.
3.  **Jump**: CPU jumps to Kernel Exception Handler (`do_int3`).

---

## 4. The Hijack (Inside The Handler)

**Kernel Execution**:
1.  Kernel sees the Trap was caused by our `kretprobe`.
2.  Kernel calls `pre_handler_kretprobe`.
3.  **Crucial Step 1 (Allocation)**: Kernel allocates an instance `ri` (Backpack).
4.  **Crucial Step 2 (Reading History)**:
    *   Kernel looks at saved `pt_regs->rsp` (`0x9000`).
    *   Kernel reads RAM at `0x9000`.
    *   Value Found: **0x105**.
5.  **Crucial Step 3 (Preservation)**:
    *   Kernel saves `0x105` into `ri->ret_addr`.
6.  **Crucial Step 4 (The Hack)**:
    *   Kernel **OVERWRITES** RAM at `0x9000`.
    *   New Value: **0xTRAMPOLINE** (Address of `kretprobe_trampoline`).

**State at T=3 (RAM Modified)**:
*   RAM[`0x9000`]: **0xTRAMPOLINE** (Previously 0x105).

---

## 5. The Execution (Back to Normal?)

**Kernel Execution**:
1.  Kernel restores registers from `pt_regs`.
2.  Kernel executes the *Original Instruction* (from `0x1000`) in a buffer.
3.  Kernel jumps back to `0x1001` (Instruction 2 of function).

**State at T=4 (Function Running)**:
*   `__sys_socket` executes normally.
*   It calculates return values...
*   It finishes.

---

## 6. The Return (The Trap Triggered)

**State at T=5 (Function executes RET)**:
1.  CPU reads Stack at `0x9000`.
2.  CPU expects `0x105`.
3.  CPU finds **0xTRAMPOLINE**.
4.  CPU updates `RIP` to `0xTRAMPOLINE`.

**State at T=6 (Inside Trampoline)**:
1.  Trampoline Code pauses execution.
2.  Finds the backpack `ri`.
3.  Calls our `ret_handler` (The Exit Probe).
    *   We verify args, calculate latency, check `regs->ax`.
4.  Trampoline reads `ri->ret_addr` (`0x105`).
5.  Trampoline jumps to `0x105`.

**Result**: User Space resumes, oblivious to the fact it just took a detour through our logging logic.


---


# 6. Probe A Findings (KASLR & Addresses)
# PROBE 1: THE ENTRY POINT & KASLR PROOF

## 1. THE TARGET
*   **Function**: `__sys_socket`
*   **Source**: `net/socket.c`
*   **Role**: The first C function in the kernel that handles the socket system call logic.

## 2. THE ADDRESSES (Evidence of KASLR)

### A. LIVE (Runtime)
*   **Command**: `sudo grep -w "_text" /proc/kallsyms`
*   **Address**: `0xffffffff92a00000` (Kernel Start in RAM)
*   **Origin**: The actual memory location where the kernel is running RIGHT NOW.

### B. STATIC (Compile-time)
*   **Command**: `sudo grep -w "_text" /boot/System.map-$(uname -r)`
*   **Address**: `0xffffffff81000000` (Compiled Start)
*   **Origin**: The address assigned by the linker during compilation.

## 3. THE DIFFERENCE (Offset Calculation)
*   **Math**: `0x92a00000` - `0x81000000` = **`0x11a00000`**
*   **Verification**: 
    - `0x93bcfaf0` (__sys_socket LIVE) - `0x821cfaf0` (__sys_socket STATIC) = **`0x11a00000`**
*   **Conclusion**: The Kernel Randomly Slid itself by `0x11a00000` bytes (282 MB) at boot time.
*   **Why?**: Security (ASLR/KASLR) to prevent exploit targeting.

## 4. PROBING STRATEGY
*   We use **Symbol Name** (`"__sys_socket"`) for `kprobes`.
*   The Kernel automatically resolves `Symbol` -> `Live Address`.
*   This bypasses the need to manually calculate the offset.

## 5. REPRODUCIBILITY LOG
The following commands were run to verify this proof:
1. `uname -r` -> `6.14.0-37-generic`
2. `sudo apt-get install linux-image-$(uname -r)-dbgsym`
3. `sudo grep -w "__sys_socket" /proc/kallsyms`
4. `sudo gdb -batch -ex "print &__sys_socket" /usr/lib/debug/boot/vmlinux-...`
5. `sudo cat /boot/config-$(uname -r) | grep CONFIG_PHYSICAL_START`
# AXIOMATIC EXPLANATION: KERNEL MEMORY AND KASLR

## 01. THE STARTING POINT (Physical Reality)
*   **Axiom 1**: Computers have RAM (Random Access Memory).
*   **Axiom 2**: RAM is a linear array of bytes, numbered `0` to `MEM_MAX`.
*   **Axiom 3**: The CPU executes instructions stored in these bytes.

## 02. THE PHYSICAL ADDRESS (The Hardware View)
*   **Definition**: The number printed on the "mailbox" of a RAM byte.
*   **Example**: `0x1000000` is the 16,777,216th byte in your RAM stick.
*   **Fact**: The Kernel (the OS Core) is loaded into RAM at boot.
*   **Config**: `CONFIG_PHYSICAL_START` tells the bootloader to load the kernel at `0x1000000` (16MB mark).

## 03. THE VIRTUAL ADDRESS (The Software View)
*   **Axiom 4**: The CPU has a "Translator" called the MMU (Memory Management Unit).
*   **Function**: Software uses Fake Addresses (Virtual) -> MMU -> Real Addresses (Physical).
*   **Mapping**: The Kernel decides to live at the "Top" of the address space (`0xffffffff...`).
*   **Static Mapping**: The compiler (`gcc`/linker) assumes the kernel starts at `0xffffffff81000000`.
    - `0xffffffff80000000` + `0x1000000` (Physical Start) = `0xffffffff81000000` (approx logic).

## 04. THE SECURITY PROBLEM
*   **Fact**: If the kernel ALWAYS starts at `...81000000`, hackers know exactly where functions are.
*   **Attack**: "Jump to `...81000000` + 50 to call `root_access()`".
*   **Solution**: Move the kernel to a random spot every time it boots.

## 05. KASLR (Kernel Address Space Layout Randomization)
*   **Action**: At boot, the generic loader picks a random number (`OFFSET`).
*   **New Mapping**: `Virtual_Addr = Static_Addr + OFFSET`.

## 06. THE EVIDENCE (Your System)
*   **Step A (Static)**: We checked `System.map`.
    - Compiler said: "I start at `0xffffffff81000000`".
*   **Step B (Live)**: We checked `/proc/kallsyms` (The MMU's current truth).
    - CPU said (MMU): "I am actually at `0xffffffff92a00000`".
*   **Step C (Derivation)**:
    - `92a00000` - `81000000` = `11a00000`.
    - **∴ OFFSET = `0x11a00000`**.

## 07. RELEVANCE TO PROBING
*   **Problem**: If you tell `kprobe` to spy on `0xffffffff81000000`, you spy on EMPTY SPACE (Crash/Silence).
*   **Correction**: You must add the `OFFSET` (`0x11a00000`) to find the REAL code.
*   **Automation**: The `kprobe` system inside the kernel knows its own offset, so we just give it the NAME (`_text` or `__sys_socket`) and it handles the math.

## 08. FINAL EQUATION
*   `Live_Addr(__sys_socket)` // `0xffffffff93bcfaf0`
*   **EQUALS** (=)
*   `Static_Addr(__sys_socket)` // `0xffffffff821cfaf0`
*   **PLUS** (+)
*   `Run_Time_Offset` // `0x11a00000`

**(Mathematics Verified by GDB and Kallsyms)**

## 09. RAW EVIDENCE (System.map)
The following are the static addresses extracted directly from the disk file `/boot/System.map-6.14.0-37-generic`:

```text
ffffffff821cfaf0 T __sys_socket
ffffffff81000000 T _text
```
*(These match the "Static" values used in the equation above.)*

## 10. LIVE EXECUTION VERIFICATION (Probe 1)
We compiled and ran the `socket_probe.c` module alongside `socket_test.c`:

```text
[ 3160.974378] [PROBE] Single-Probe Driver (__sys_socket) Loaded.
[ 3160.979220] [PROBE] __sys_socket ENTRY: family=2, type=1, protocol=0
```

*   **Family 2**: `AF_INET` (Verified)
*   **Type 1**: `SOCK_STREAM` (Verified)
*   **Protocol 0**: `Default` (Verified)

This proves that `socket_probe.c` successfully intercepted the exact system call made by `socket_test`.
