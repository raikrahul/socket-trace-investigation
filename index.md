---
layout: default
---

# SOCKET AXIOMS: CHRONOLOGICAL EXECUTION TRACE

## PHASE 0: COMPILE TIME (THE PREPROCESSOR)
[LESSON 06: THE PREPROCESSOR (TRUTH)](lessons/06_preprocessor_truth.md)
**Time**: T-minus 1.
**Action**: `#include <sys/socket.h>`
**Result**: `AF_INET -> 2`, `SOCK_STREAM -> 1`.

## PHASE 1: USER SPACE RUNTIME (THE REGISTERS)
[LESSON 07: THE REGISTERS (ABI TRUTH)](lessons/07_register_truth.md)
**Time**: T=0.
**Action**: `mov $2, %rdi`, `mov $1, %rsi`.
**Result**: CPU Registers prepared for ABI.

## PHASE 2: THE KERNEL TRAP (THE TRANSITION)
[LESSON 08: THE SYSCALL TRAP](lessons/08_syscall_trap.md)
**Time**: T+1.
**Action**: `mov $41, %rax`, `syscall`.
**Result**: CPU Mode Switch (CPL3 -> CPL0).

## PHASE 3: KERNEL ENTRY (THE DISPATCH)
[LESSON 05: THE FULL AXIOMATIC TRACE](lessons/05_axiomatic_trace.md)
**Time**: T+2.
**Action**: `do_syscall_64` -> `__sys_socket`.
**Result**: Kernel begins executing C code.

## PHASE 4: OBJECT ALLOCATION (THE GEOMETRY)
[LESSON 01: THE SLAB GEOMETRY](lessons/01_slab_geometry.md)
**Time**: T+3.
**Action**: `kmem_cache_alloc(sock_inode_cache)`.
**Result**: 832 bytes allocated. `Socket` at +0, `Inode` at +128.

## PHASE 5: PROTOCOL INITIALIZATION (THE LOOKUP)
[LESSON 03: PROTOCOL COLLISION](lessons/03_protocol_collision.md)
**Time**: T+4.
**Action**: Walk `inetsw` linked list.
**Result**: Match `type=1, protocol=0` -> TCP (6).

## PHASE 6: DEEP MEMORY (THE HIDDEN COST)
[LESSON 11: THE RUSSIAN DOLL PUZZLE](lessons/11_russian_dolls.md)
**Time**: T+5.
**Action**: `sk_alloc`.
**Result**: `tcp_sock` (2360 bytes) allocated and cast to `sock`.

## PHASE 7: RETURN TO USER (THE ARITHMETIC)
[LESSON 04: PRIMATE ARITHMETIC](lessons/04_primate_arithmetic.md)
**Time**: T+6.
**Action**: `fd_install`.
**Result**: File Descriptor returned to `%rax`.

---

## MANIFESTOS
[LESSON 10: THE LKML MANIFESTO (ZERO TOLERANCE)](lessons/10_lkml_manifesto.md)
[LESSON 00: THE PREFACE (FOR LLMS)](lessons/00_llm_preface.md)
[LESSON 02: THE MISTAKES LOG](lessons/02_mistakes_log.md)

## ARTIFACTS
- `proofs/` - C Code Verification.
- `machine_truth.json` - Raw Data.
- `wiki/` - Archives.
