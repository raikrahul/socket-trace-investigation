# SOCKET TRACE: AXIOMATIC VERIFICATION

**[LIVE LOG: https://raikrahul.github.io/socket-trace-investigation/](https://raikrahul.github.io/socket-trace-investigation/)**

## PURPOSE
To mathematically derive the execution path of the `socket(2, 1, 0)` system call from User Space (ASM) to Kernel Space (SLAB), verifying every constant via runtime interrogation of the Linux Kernel (v6.14.0-37).

## THE SYLLABUS: THE FIRST THREE ERAS

### [ERA I: THE DESIGN](lesson_design.html)
**"The Blueprint"**
- The VFS Contract: `inode` vs `socket` vs `file`.
- The Allocation Strategy: SLAB vs SLUB.
- The 768-byte math behind the `socket_alloc` struct.

### [ERA II: THE COMPILATION](lesson_compile.html)
**"The Translation"**
- The Macro Expansion of `SYSCALL_DEFINE3`.
- The Assembly Output: `entry_SYSCALL_64`.
- The Control Flow Graph (CFG) from `sys_socket` to `__sys_socket`.

### [ERA III: BOOT TIME](lesson_boot.html)
**"The Awakening"** 
- The `sock_init` sequence.
- **Sysctl**: Creating `/proc/sys/net`.
- **Memory**: Creating the `skbuff_head_cache`.
- **VFS**: Registering `sockfs` (O(N) Complexity Proof).
- **Mount**: Creating the `sock_mnt` global anchor.

## VERIFIED AXIOMS
All data in this repository is derived from the accompanying C probes in `proofs/`.
- **System Call**: `__NR_socket` (41) via `MSR_LSTAR`.
- **Dispatch**: `inetsw[1]` collision resolution.
 
## REPRODUCTION
```bash
git clone https://github.com/raikrahul/socket-trace-investigation.git
cd socket-trace-investigation/proofs
make
sudo ./run_probes.sh
```
