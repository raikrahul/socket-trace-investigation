# RUTHLESS ERROR LOG

## 01. THE INETSW SCANDAL
LINE: `Lookup: inetsw[1] = tcp_protosw` (part1.md)
ERROR: Assumed direct array access.
REALITY: `inetsw` is a `struct list_head` array. It contains HEADERS, not PROTOCOLS.
MISSED: The linked list traversal loop in `inet_create`.
PREVENTION: Never assume a kernel array is a direct lookup table. Always check the type. `list_head` = CHAIN.

## 02. THE ZERO PROTOCOL LIE
LINE: `IPPROTO_IP = 0 ... Why: 0 = default protocol` (part1.md)
ERROR: Magic number thinking.
REALITY: User passes 0. Kernel explicitly checks `if (protocol == answer->protocol)` OR wildcard logic. TCP *registers* itself.
MISSED: The loop logic:
```c
if (protocol == answer->protocol) break;
if (IPPROTO_IP == protocol) protocol = answer->protocol; break;
```
PREVENTION: Trace the IF statements. Don't just assert "It defaults".

## 03. THE SLAB ALLOCATOR GLOSS-OVER
LINE: `Call: kmem_cache_alloc ... Returns: struct sock*`
ERROR: Ignored the `slab` cache name and mechanics.
REALITY: `tcp_prot.slab` is initialized specifically for TCP. It's not just "a slab".
PREVENTION: Find the `kmem_cache_create` call for the protocol.

## 04. THE VERIFICATION GAP
LINE: `Result: ✓` (part1.md)
ERROR: Used checkmark without showing memory dump proof for intermediate pointers.
REALITY: `inetsw[1]` address was never shown.
PREVENTION: Dump the pointer address of the list head and the first node.

## 05. AESTHETIC SLOPPINESS
LINE: Usage of Headers (H1, H2) and Adjectives ("Giant", "King of Networking").
ERROR: "The user is a primate who understands counting". Adjectives confuse the primate.
REALITY: Primate needs: `Input -> Process -> Output`.
PREVENTION: Delete all adjectives. Use monospaced font. Use 01, 02, 03 numbering.
