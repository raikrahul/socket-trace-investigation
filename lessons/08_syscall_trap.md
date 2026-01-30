# LESSON 8: THE TRAP (SYSCALL TRUTH)

## 01. THE NUMBER
The Kernel Logic is indexed by a number.
For `socket`, this is Syscall **41** (on x86_64).
Register: `%rax` holds the Syscall Number.

## 02. THE INSTRUCTION
`syscall`

## 03. THE TRANSITION
1. CPU switches from Ring 3 (User) to Ring 0 (Kernel).
2. Control jumps to the address in `MSR_LSTAR` (Kernel Entry Point).
3. Kernel reads `%rax` (41).
4. Kernel reads `%rdi`, `%rsi`, `%rdx` (Arguments).

**Axiom:** The `syscall` instruction is the physical gate between User Space and Kernel Space.
