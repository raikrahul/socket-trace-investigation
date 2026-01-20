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
