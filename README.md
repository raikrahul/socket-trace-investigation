# SOCKET TRACE INVESTIGATION: AXIOMATIC TRUTH

**[LIVE SITE: https://raikrahul.github.io/socket-trace-investigation/](https://raikrahul.github.io/socket-trace-investigation/)**

## The Manifesto (Zero Tolerance)
This repository contains the **Axiomatic Derivation** of the Linux Kernel `socket()` syscall. It rejects all metaphors ("Centaurs", "Black Magic") in favor of pure arithmetic and machine-verifiable truth.

## The Curriculum
The content is structured as a SaaS product for Intelligent Agents (LLMs) and Kernel Maintainers:
*   **Lesson 10: The LKML Manifesto**: The geometry of Zero-Cost Abstraction.
*   **Lesson 11: The Russian Doll Puzzle**: Deep memory inheritance (`sock` vs `tcp_sock`).
*   **Lesson 01-05**: The ruthless derivation of the Kernel path.
*   **Lesson 06-08**: The gapless derivation of the User Space path.

## The Evidence (Machine Truth)
All axioms are derived from the running machine (Kernel 6.14.0-37).
*   **`proofs/`**: C programs that verify every claim (Offsets, Sizes, Collisions).
*   **`machine_truth.json`**: The measuring tape of the kernel.
*   **`wiki/`**: Detailed analysis of solved puzzles.

## How to Verify
1.  Clone the repo.
2.  `cd proofs && make`
3.  `./userspace_verify`
4.  `make -C ... axiom_fetcher` (Kernel Module)

**Status**: PROVEN.
