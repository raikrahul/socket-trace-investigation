# THE HARDER PUZZLE: INETSW LINKED LIST

## 01. THE PROBLEM
Axiom 1: `socket(AF_INET, SOCK_STREAM, 0)` returns a TCP socket.
Axiom 2: `SOCK_STREAM` is an integer index (1).
Naive Assumption: `inetsw[1]` is a direct pointer to TCP.
Contradiction: MPTCP and SCTP also support `SOCK_STREAM`.
Question: How does the kernel disambiguate?

## 02. THE DATA STRUCTURE
Source: `net/ipv4/af_inet.c` (Static Symbol)
Axiom: `inetsw` is `struct list_head inetsw[SOCK_MAX];`
Reality: It is an Array of Linked Lists (Hash Table buckets).

## 03. THE PROOF (Runtime Probe v2)
We walked `inetsw[1]` on Kernel 6.14.0-37-generic.
Address: `0xffffffffb1bcd040`

### Node 1 (TCP)
- Type: 1 (SOCK_STREAM)
- Protocol: 6 (IPPROTO_TCP)
- Name: "TCP"
- Flags: 0x6 (INET_PROTOSW_PERMANENT | INET_PROTOSW_ICSK)

### Node 2 (MPTCP)
- Type: 1 (SOCK_STREAM)
- Protocol: 262 (MPTCP)
- Name: "MPTCP"
- Flags: 0x4 (INET_PROTOSW_ICSK)

## 04. THE MATCHING LOGIC
Type is used as Hash Key: `inetsw[type]`
Loop:
1. Iterate list.
2. `if (protocol == answer->protocol)`: EXACT MATCH.
3. `if (IPPROTO_IP == protocol)`: WILDCARD MATCH (User passed 0).
   - TCP registers with `protocol=6` but allows wildcard?
   - NO. The loop logic permits "first permanent" or specific fallback.
   - Actually: `if (IPPROTO_IP == protocol)` -> Take first answer.

## 05. CONCLUSION
It is NOT a direct lookup.
It IS a Linear Scan of a Linked List.
Efficiency: O(N) where N is small (2 protocols).
Correctness: Allows protocol overloading on socket types.
