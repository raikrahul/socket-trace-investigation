# SOCKET DERIVATION: MISTAKES AND CONFUSIONS

## YOUR CONFUSIONS (DOCUMENTED)

### CONFUSION 1: vfsmount is socket related
YOUR THOUGHT: vfsmount is used by socket code.
REALITY: vfsmount is generic VFS. Used by ALL 46 filesystems.
CORRECTION: sock_mnt points to ONE vfsmount struct. That struct is generic.

### CONFUSION 2: C has instances/objects
YOUR THOUGHT: "vfsmount instance #47"
REALITY: C has no instances. Only structs at RAM addresses.
CORRECTION: vfsmount struct at address 0xffff888... (not "instance")

### CONFUSION 3: socket exists in the chain
YOUR THOUGHT: sock_mnt -> vfsmount -> super_block -> ... where is socket?
REALITY: Socket does NOT exist in that chain.
CORRECTION: Chain finds FUNCTION. Function CREATES socket. Socket exists AFTER.

### CONFUSION 4: ext4_sops should differ per drive
YOUR THOUGHT: 3 ext4 drives should have 3 different ext4_sops.
REALITY: CODE is same. DATA differs.
CORRECTION: All ext4 use same ext4_sops (code). super_block->s_bdev differs (data).

### CONFUSION 5: things happen when socket() is called
YOUR THOUGHT: structures created when I call socket().
REALITY: structures created at BOOT (sock_init). Socket() just USES them.
CORRECTION: Boot created pool, vfsmount, sockfs_ops. They waited for you.

### CONFUSION 6: VFS knows about sockets
YOUR THOUGHT: VFS does socket-specific things.
REALITY: VFS knows NOTHING about sockets.
CORRECTION: VFS reads generic structs. Ends up calling socket-specific function.

---

## TIMELINE OF YOUR CONFUSIONS

```
YOUR ORIGINAL UNDERSTANDING (WRONG):
socket() call -> creates structures -> allocates memory

ACTUAL SEQUENCE:
1. COMPILE TIME: sockfs_ops created with function addresses
2. BOOT TIME: sock_init() creates pool, vfsmount, super_block
3. MANY MINUTES PASS: structures wait
4. YOU LOG IN: structures still waiting
5. YOU WRITE CODE: structures still waiting
6. socket() CALL: uses pre-existing structures, allocates from pool
```

---

## ORTHOGONAL QUESTIONS TO YOUR SLOPPY BRAIN

Q1: If VFS is generic, why does socket code call VFS functions?
A1: Sockets are implemented AS a filesystem. sockfs uses VFS infrastructure.

Q2: If structures were created at boot, why do we need the callback chain?
A2: Boot created pointers. Runtime follows them.

Q3: Why 720 bytes and not 592 + 128 separately?
A3: Allocation overhead. One allocation cheaper than two.

Q4: Why is sockfs invisible in /proc/mounts?
A4: kern_mount() creates kernel-internal mount. No mount point.

Q5: If sock_mnt is global, can any code read it?
A5: Only kernel code. User space cannot access kernel RAM.

Q6: What happens if sock_init() fails?
A6: Kernel panic. Socket subsystem mandatory.

---

## WHAT YOU SHOULD HAVE ASKED FIRST

1. WHEN were structures created? (BOOT, not socket() call)
2. WHERE is socket-specific code? (sock_alloc_inode, not VFS)
3. WHAT is the purpose of the chain? (find function, not do work)
4. WHY reuse VFS? (infrastructure sharing, not socket needs)

---

## YOUR LEARNING PATH (CORRECTED)

```
STEP 1: COMPILATION (months ago)
  - Struct sizes computed
  - Function addresses assigned
  - sockfs_ops table created

STEP 2: BOOT (minutes before you log in)
  - sock_init() runs
  - Pool created (720-byte blocks)
  - vfsmount, super_block allocated
  - sock_mnt, sock_inode_cachep set

STEP 3: WAITING (minutes to hours)
  - Structures exist in RAM
  - No sockets yet
  - Pool has 0 allocated blocks

STEP 4: YOUR CODE RUNS
  - socket(2, 1, 0) called
  - Chain followed
  - Block allocated from pool
  - Socket NOW EXISTS
```

---

## KEY INSIGHT YOU MISSED

THE STRUCTURES WERE WAITING FOR YOU.

You thought: "I call socket(), things happen."
Reality: "Things already happened. socket() uses them."

---

### MISTAKE 7: NON-AXIOMATIC FORMATTING
- **User Thought:** "Professional and sellable" means blog post prose and visual tables.
- **Reality:** Primate-logic requires one-fact-per-line axiomatic derivation.
- **Correction:** Revert to pure numbered lines where Line N only uses knowledge from Line 1 to N-1.

### MISTAKE 8: FORWARD REFERENCES
- **User Thought:** Can mention future steps (e.g., "we will see in Block 8").
- **Reality:** Rule 16: NO forward references.
- **Correction:** Each fact must stay within its linear dependency chain.

---

## REJECTED CONTENT LOG
- Rejected: Split-column tables (Violates "1 fact per line" simplified visual flow).
- Rejected: Prose blocks (Violates "input → computation → output" rule).
- Rejected: <h1> and <h2> headers (Violates "0 heading" rule).

---

## PROOFS GATHERED
- [✓] sizeof(socket_alloc) = 720
- [✓] offset(mnt_sb) = 8
- [✓] offset(s_op) = 48
- [✓] offset(alloc_inode) = 0
- [✓] sock_mnt = 0xffffffffbd0767a0
- [✓] sock_alloc_inode = 0xffffffffbb7ccda0
| CLAIM | PROOF METHOD | RESULT |
|-------|--------------|--------|
| 46 vfsmount structs | mount \| wc -l | 46 |
| sockfs_ops at 0xffffffffbbd6e740 | kallsyms | verified |
| sock_alloc_inode at 0xffffffffbb7ccda0 | kallsyms | verified |
| mnt_sb at offset 8 | offsetof() | 8 bytes |
| s_op at offset 48 | offsetof() | 48 bytes |
| alloc_inode at offset 0 | offsetof() | 0 bytes |
| 1541 sockets active | /proc/net/sockstat | verified |
| sockfs is nodev | /proc/filesystems | nodev sockfs |
| kernel version | uname -r | 6.14.0-37-generic |
