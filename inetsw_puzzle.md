# THE HARDER PUZZLE: THE INETSW LINKED LIST

You were right. I was sloppy. I said "Lookup" as if it was `Array[Index]`.
It is `Array[Index] -> Linked List -> Scan -> Match`.

## 1. THE STRUCT (The Nodes)
Source: `include/net/protocol.h:76`
```c
struct inet_protosw {
    struct list_head list;       // <-- THE CHAIN MECHANISM
    unsigned short   type;       // Key 1 (SOCK_STREAM)
    unsigned short   protocol;   // Key 2 (IPPROTO_TCP)
    struct proto     *prot;      // The Payload
    const struct proto_ops *ops; // The Ops
    unsigned char    flags;
};
```
**Axiom**: Every protocol (TCP, UDP, SCTP) is a "Node" in a chain.

## 2. THE ARRAY (The Buckets)
Source: `net/ipv4/af_inet.c:128`
```c
static struct list_head inetsw[SOCK_MAX];
```
**Axiom**: This is a Hash Table (using Type as the hash), resolving collisions via Linked Lists.

## 3. THE COLLISION (The Problem)
What happens if two protocols want `SOCK_STREAM`?
*   TCP wants `SOCK_STREAM` (1).
*   SCTP wants `SOCK_STREAM` (1).

They BOTH go into `inetsw[1]`.
`inetsw[1] -> Node(SCTP) -> Node(TCP) -> NULL`

## 4. THE REAL LOOKUP (The Crack)
Source: `net/ipv4/af_inet.c:271`
```c
list_for_each_entry_rcu(answer, &inetsw[sock->type], list) {
    // LOOP START
    err = 0;

    // RULE 1: Check Protocol Match
    if (protocol == answer->protocol) {
        if (protocol != IPPROTO_IP) // Exact match!
            break;
    } 
    
    // RULE 2: Check Wildcard Match (argument 0)
    else {
        if (IPPROTO_IP == protocol) { // User passed 0
            protocol = answer->protocol; // Take default
            break;
        }
        if (IPPROTO_IP == answer->protocol) // Catch-all
            break;
    }
    // LOOP END
}
```

## 5. THE TRACE (socket(2, 1, 0)) REVISITED
1.  **Arguments**: `family=2`, `type=1`, `protocol=0`.
2.  **Bucket**: `inetsw[1]`.
3.  **Iteration 1 (SCTP)**:
    *   Node says: `protocol = IPPROTO_SCTP (132)`.
    *   User asked: `0`.
    *   Match? NO. (SCTP is not a wildcard default).
4.  **Iteration 2 (TCP)**:
    *   Node says: `protocol = IPPROTO_TCP (6)`.
    *   User asked: `0`.
    *   **WILDCARD MATCH!** (Logic: TCP is allowing itself to be the default).
    *   Break Loop.

## 6. THE TRACE (socket(2, 1, 132)) SCTP CALL
1.  **Arguments**: `family=2`, `type=1`, `protocol=132`.
2.  **Bucket**: `inetsw[1]`.
3.  **Iteration 1 (SCTP)**:
    *   Node: `132`.
    *   User: `132`.
    *   **EXACT MATCH!**
    *   Break Loop.
    *   Result: You get SCTP, not TCP.

## CONCLUSION
One Array Access (`inetsw[1]`) was WRONG.
It is **Array Access + Linked List Traversal + Protocol Matching Logic**.
This allows different protocols to coexist on the same socket type.
