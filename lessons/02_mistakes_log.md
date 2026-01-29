# LESSON 2: THE MISTAKES LOG (EASY)

## 01. THE ZERO PROTOCOL ERROR
Mistake: Believing `protocol=0` means "Default" by magic.
Truth: `0` is `IPPROTO_IP`. The kernel checks `if (protocol == IPPROTO_IP)` and *replaces* it with the registered protocol (e.g., 6).

## 02. THE ARRAY ERROR
Mistake: `inetsw[SOCK_STREAM]` is a pointer to TCP.
Truth: `inetsw[SOCK_STREAM]` is a List Head. It points to a chain of nodes.

## 03. THE STORY ERROR
Mistake: Using metaphors like "The Centaur" or "Register Heist".
Truth: Code operations are arithmetic and data movement.
- `Centaur` -> `struct socket_alloc`
- `Heist` -> `mov` instruction
**Axiom:** Metaphors obscure the input-output logic.
