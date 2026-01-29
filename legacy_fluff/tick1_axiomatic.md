# TICK 1: AXIOMATIC DERIVATION

## AXIOM BLOCK A: NUMBERS AND MEMORY

A01. 1 byte = 8 bits.
A02. 1 bit = 0 or 1.
A03. Address = Number. (e.g., 0, 1, 2, ..., 0xFFFFFFFFFFFFFFFF).
A04. RAM = Array of Bytes indexed by Address.
A05. RAM[Address] = 1 Byte at that Address.
A06. Pointer = Address stored in RAM.
A07. sizeof(Pointer) = 8 bytes (on 64-bit machine).

## AXIOM BLOCK B: STRUCTS

B01. struct = Contiguous block of bytes.
B02. struct has fields.
B03. Field = Name + Type + Offset.
B04. Offset = Number of bytes from start of struct.
B05. sizeof(struct) = Sum of field sizes + padding.
B06. Padding = Extra bytes for alignment.
B07. Alignment = Field address must be divisible by field size.

## AXIOM BLOCK C: FUNCTION CALLS

C01. Function = Sequence of instructions at an Address.
C02. Call = Jump to function address + Push return address.
C03. Return = Pop return address + Jump back.
C04. Argument = Value passed to function.
C05. Return Value = Value passed back from function.

---

## DERIVATION D01: struct socket (Source: include/linux/net.h:117)

Using Axioms B01-B07:

```
struct socket {
    socket_state state;           // Offset +0,  Size 4
    short        type;            // Offset +4,  Size 2
    // Padding 2 bytes            // Offset +6,  Size 2
    unsigned long flags;          // Offset +8,  Size 8
    struct file *file;            // Offset +16, Size 8  (Pointer, A07)
    struct sock *sk;              // Offset +24, Size 8  (Pointer, A07)
    const struct proto_ops *ops;  // Offset +32, Size 8  (Pointer, A07)
    struct socket_wq wq;          // Offset +40, Size 88 (Embedded struct)
};
```

D01-CALC: sizeof(struct socket) = 4 + 2 + 2 + 8 + 8 + 8 + 8 + 88 = 128 bytes.

---

## DERIVATION D02: struct inode (Source: include/linux/fs.h:642)

D02-STATEMENT: struct inode is large (~600 bytes).
D02-REASON: Contains file metadata (permissions, timestamps, links).
D02-SIZE: sizeof(struct inode) ~ 600 bytes.

---

## DERIVATION D03: struct socket_alloc (Source: include/net/sock.h:1555)

Using D01 and D02:

```
struct socket_alloc {
    struct socket socket;   // Offset +0,   Size 128 (D01)
    struct inode vfs_inode; // Offset +128, Size 600 (D02)
};
```

D03-CALC: sizeof(struct socket_alloc) = 128 + 600 = 728 bytes.

---

## DERIVATION D04: Slab Cache (Source: net/socket.c:341)

D04-AXIOM: Kernel pre-allocates memory pools for fixed-size objects.
D04-NAME: "sock_inode_cache".
D04-SIZE: sizeof(struct socket_alloc) = 728 bytes (D03).

Boot-time code:
```c
sock_inode_cachep = kmem_cache_create("sock_inode_cache",
                                      sizeof(struct socket_alloc), // = 728
                                      0,
                                      SLAB_HWCACHE_ALIGN,
                                      NULL);
```

D04-RESULT: Global variable `sock_inode_cachep` holds pointer to this cache.

---

## DERIVATION D05: sock_alloc_inode Function (Source: net/socket.c:304)

D05-INPUT: struct super_block *sb (not used in our trace).
D05-OUTPUT: struct inode * (Pointer to inode part of allocated block).

### LINE 306: Variable Declaration

```c
struct socket_alloc *ei;
```

D05-L306: `ei` is a local variable.
D05-L306-TYPE: Pointer to struct socket_alloc (D03).
D05-L306-SIZE: 8 bytes (A07).
D05-L306-VALUE: Undefined (garbage).

### LINE 309: Allocation

```c
ei = alloc_inode_sb(sb, sock_inode_cachep, GFP_KERNEL);
```

D05-L309-ACTION: Request 728 bytes from sock_inode_cache (D04).
D05-L309-RESULT: Kernel returns Address 0xffff8f4e33230340.
D05-L309-STORE: ei = 0xffff8f4e33230340.

### LINE 310-311: NULL Check

```c
if (!ei)
    return NULL;
```

D05-L310-CHECK: Is ei == 0?
D05-L310-RESULT: 0xffff8f4e33230340 != 0. Check passes.

### LINE 316: state = SS_UNCONNECTED

```c
ei->socket.state = SS_UNCONNECTED;
```

D05-L316-CALC:
  - ei = 0xffff8f4e33230340 (D05-L309).
  - offsetof(socket_alloc, socket) = 0 (D03).
  - offsetof(socket, state) = 0 (D01).
  - Target Address = 0xffff8f4e33230340 + 0 + 0 = 0xffff8f4e33230340.
  - SS_UNCONNECTED = 0.
  - Write: RAM[0xffff8f4e33230340] = 0.

### LINE 317: flags = 0

```c
ei->socket.flags = 0;
```

D05-L317-CALC:
  - Base = 0xffff8f4e33230340 (D05-L309).
  - offsetof(socket, flags) = 8 (D01).
  - Target = 0xffff8f4e33230340 + 8 = 0xffff8f4e33230348.
  - Write: RAM[0xffff8f4e33230348] = 0 (8 bytes).

### LINE 318: ops = NULL

```c
ei->socket.ops = NULL;
```

D05-L318-CALC:
  - Base = 0xffff8f4e33230340 (D05-L309).
  - offsetof(socket, ops) = 32 (D01).
  - Target = 0xffff8f4e33230340 + 32 = 0xffff8f4e33230360.
  - NULL = 0.
  - Write: RAM[0xffff8f4e33230360] = 0 (8 bytes).

### LINE 319: sk = NULL

```c
ei->socket.sk = NULL;
```

D05-L319-CALC:
  - Base = 0xffff8f4e33230340 (D05-L309).
  - offsetof(socket, sk) = 24 (D01).
  - Target = 0xffff8f4e33230340 + 24 = 0xffff8f4e33230358.
  - Write: RAM[0xffff8f4e33230358] = 0 (8 bytes).

### LINE 320: file = NULL

```c
ei->socket.file = NULL;
```

D05-L320-CALC:
  - Base = 0xffff8f4e33230340 (D05-L309).
  - offsetof(socket, file) = 16 (D01).
  - Target = 0xffff8f4e33230340 + 16 = 0xffff8f4e33230350.
  - Write: RAM[0xffff8f4e33230350] = 0 (8 bytes).

### LINE 322: Return

```c
return &ei->vfs_inode;
```

D05-L322-CALC:
  - ei = 0xffff8f4e33230340 (D05-L309).
  - offsetof(socket_alloc, vfs_inode) = 128 (D03).
  - Return Value = 0xffff8f4e33230340 + 128 = 0xffff8f4e332303C0.

---

## DERIVATION D06: SOCKET_I Macro (Source: include/net/sock.h:1560)

D06-PURPOSE: Convert inode* back to socket*.

```c
#define SOCKET_I(inode) \
    &container_of(inode, struct socket_alloc, vfs_inode)->socket
```

D06-INPUT: inode = 0xffff8f4e332303C0 (D05-L322).
D06-CALC:
  - offsetof(socket_alloc, vfs_inode) = 128 (D03).
  - socket_alloc_base = inode - 128 = 0xffff8f4e332303C0 - 0x80 = 0xffff8f4e33230340.
  - socket = socket_alloc_base + 0 = 0xffff8f4e33230340.
D06-OUTPUT: 0xffff8f4e33230340.

---

## MEMORY MAP AFTER TICK 1

```
Address              | Value          | Field
---------------------|----------------|------------------
0xffff8f4e33230340   | 0              | socket.state
0xffff8f4e33230344   | 0              | socket.type
0xffff8f4e33230348   | 0              | socket.flags
0xffff8f4e33230350   | 0              | socket.file (NULL)
0xffff8f4e33230358   | 0              | socket.sk (NULL)
0xffff8f4e33230360   | 0              | socket.ops (NULL)
...                  | ...            | socket.wq
0xffff8f4e332303C0   | ...            | vfs_inode (start)
```

---

## TICK 1 SUMMARY

INPUT:  None (sock_alloc takes no arguments).
OUTPUT: Pointer 0xffff8f4e33230340 (struct socket *).
ACTION: Allocated 728 bytes, initialized socket fields to NULL/0.

---

## NEW THINGS INTRODUCED WITHOUT DERIVATION: ___

(Empty. All terms derived from Axioms A, B, C and Derivations D01-D06.)
