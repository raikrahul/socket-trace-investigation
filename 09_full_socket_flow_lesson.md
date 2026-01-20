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
