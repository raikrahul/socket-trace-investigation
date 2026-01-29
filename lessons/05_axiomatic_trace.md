# LESSON 5: THE FULL AXIOMATIC TRACE (DIFFICULT)

## 01. INPUT
`socket(2, 1, 0)`.

## 02. SYSTEM CALL (Level 0)
`__sys_socket(2, 1, 0)` called.
Allocates `struct socket` (State=SS_UNCONNECTED).

## 03. DISPATCH (Level 1)
Scan `inetsw[1]`.
Linear search finds `TCP` (Prot=6).
`sock->ops` set to `inet_stream_ops`.

## 04. ALLOCATION (Level 2)
`sock_alloc()` called.
Allocates `slab` object (832 bytes physical).
Returns `A` (Socket Address).
Computes `A + 128` (Inode Address) for VFS.

## 05. LINKAGE (Level 3)
`inet_create()` called.
Allocates `struct sock` (TCP Engine).
Links:
- `socket->sk = sock`
- `sock->sk_socket = socket`

## 06. INSTALLATION (Level 4)
File Descriptor Table scan.
Finds unused index 3.
`fdt[3] = file`.
`file->private_data = socket`.

## 07. OUTPUT
Return `3`.
**Axiom:** Logic determines Memory. Memory determines Identity.
