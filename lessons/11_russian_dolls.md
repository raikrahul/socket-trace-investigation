# LESSON 11: THE RUSSIAN DOLL PUZZLE (DEPTH)

## 01. THE PROBLEM (INPUT)
User Perception: "I created a socket."
Kernel Reality: `sizeof(struct sock) = 776 bytes`.
Actual Allocation: `?? bytes`.

The Kernel passes around a `struct sock *sk`.
But `struct sock` is just the core.
What is the *actual* object?

## 02. THE DATA HEIST (FACTS)
We probed the running kernel (6.14.0-37).
- `sizeof(struct sock)` = **776 bytes**.
- `sizeof(struct inet_sock)` = **960 bytes**.
- `sizeof(struct inet_connection_sock)` = **1136 bytes**.
- `sizeof(struct tcp_sock)` = **2360 bytes**.

## 03. THE GEOMETRY (INHERITANCE)
C does not have classes. It has Embedding.
**Memory Layout of `tcp_sock`**:
```
0      [ struct sock (776) ]
776    [ struct inet_sock (+184) ]
960    [ struct inet_connection_sock (+176) ]
1136   [ struct tcp_sock (+1224) ]
-----------------------------------
Total: 2360 bytes.
```

## 04. THE CAST (ZERO COST)
The Pointers are identical.
`struct sock *sk = (struct sock *)tcp_ptr;`
`struct tcp_sock *tp = (struct tcp_sock *)sk;`

**Arithmetic:**
`address(sk) == address(tp)`.
Cost: **0 Cycles**.

## 05. THE HIDDEN COST
When you call `socket(AF_INET, SOCK_STREAM)`, the kernel allocates **2360 bytes** (TCP).
When you call `socket(AF_UNIX, SOCK_STREAM)`, it allocates something else.
But the return type is always the same generic `struct sock *`.
**Axiom:** Pointers hide geometric mass.
