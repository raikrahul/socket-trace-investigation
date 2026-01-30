# SOCKET TRACE: AXIOMATIC VERIFICATION

**[LIVE LOG: https://raikrahul.github.io/socket-trace-investigation/](https://raikrahul.github.io/socket-trace-investigation/)**

## PURPOSE
To mathematically derive the execution path of the `socket(2, 1, 0)` system call from User Space (ASM) to Kernel Space (SLAB), verifying every constant via runtime interrogation of the Linux Kernel (v6.14.0-37).

## VERIFIED AXIOMS
All data in this repository is derived from the accompanying C probes in `proofs/`.
- **System Call**: `__NR_socket` (41) via `MSR_LSTAR` (0xffffffff97800080).
- **Allocatoin**: `sock_inode_cache` (832 bytes) vs `socket_alloc` (768 bytes).
- **Dispatch**: `inetsw[1]` collision resolution (TCP vs MPTCP).
- **Inheritance**: `sizeof(tcp_sock)` (2360 bytes) > `sizeof(sock)` (776 bytes).

## REPRODUCTION
The "Linus Protocol" requires `sudo` privileges to insert kernel modules for measurement.

```bash
git clone https://github.com/raikrahul/socket-trace-investigation.git
cd socket-trace-investigation/proofs
make
sudo ./run_probes.sh
```

## STATUS
**VERIFIED**. No metaphors. No abstractions. Only machine truth.
