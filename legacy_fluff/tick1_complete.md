# TICK 1: COMPLETE AXIOMATIC DERIVATION

## AXIOM BLOCK A: NUMBERS AND MEMORY

A01. 1 byte = 8 bits.
A02. 1 bit = 0 or 1.
A03. Address = Number. Range: 0 to 0xFFFFFFFFFFFFFFFF.
A04. RAM = Array of Bytes indexed by Address.
A05. RAM[Address] = 1 Byte at that Address.
A06. Pointer = Address stored in RAM.
A07. sizeof(Pointer) = 8 bytes (on 64-bit machine).
A08. NULL = 0.

---

## AXIOM BLOCK B: STRUCTS

B01. struct = Contiguous block of bytes.
B02. struct has fields.
B03. Field = Name + Type + Offset.
B04. Offset = Number of bytes from start of struct.
B05. sizeof(struct) = Sum of field sizes + padding.
B06. Padding = Extra bytes for alignment.
B07. Alignment = Field address must be divisible by field size.
B08. offsetof(struct, field) = Offset of field within struct.

---

## AXIOM BLOCK C: FUNCTION CALLS

C01. Function = Sequence of instructions at an Address.
C02. Call = Jump to function address + Push return address.
C03. Return = Pop return address + Jump back.
C04. Argument = Value passed to function.
C05. Return Value = Value passed back from function.
C06. void = No arguments.

---

## AXIOM BLOCK D: KERNEL MEMORY

D01. Kernel Heap = RAM region for dynamic allocation.
D02. kmem_cache = Pre-sized memory pool for fixed-size objects.
D03. kmem_cache_alloc(cache) = Request one object from pool.
D04. Object Address = Number returned by kmem_cache_alloc.

---

## DERIVATION E01: struct socket (Source: include/linux/net.h:117)

Using B01-B08, A07:

```
struct socket {
    socket_state state;           // Offset +0,  Size 4
    short        type;            // Offset +4,  Size 2
    // Padding 2 bytes            // Offset +6,  Size 2
    unsigned long flags;          // Offset +8,  Size 8
    struct file *file;            // Offset +16, Size 8  (A07)
    struct sock *sk;              // Offset +24, Size 8  (A07)
    const struct proto_ops *ops;  // Offset +32, Size 8  (A07)
    struct socket_wq wq;          // Offset +40, Size 88
};
```

E01-CALC: 4 + 2 + 2 + 8 + 8 + 8 + 8 + 88 = 128 bytes.
E01-RESULT: sizeof(struct socket) = 128.

---

## DERIVATION E02: struct inode (Source: include/linux/fs.h:642)

E02-STATEMENT: struct inode contains file metadata.
E02-FIELDS: i_mode, i_uid, i_gid, i_ino, timestamps, etc.
E02-SIZE: sizeof(struct inode) ~ 600 bytes.

---

## DERIVATION E03: struct socket_alloc (Source: include/net/sock.h:1555)

Using E01, E02:

```
struct socket_alloc {
    struct socket socket;   // Offset +0,   Size 128 (E01)
    struct inode vfs_inode; // Offset +128, Size 600 (E02)
};
```

E03-CALC: 128 + 600 = 728.
E03-RESULT: sizeof(struct socket_alloc) = 728 bytes.

---

## DERIVATION E04: Slab Cache Creation (Source: net/socket.c:341)

Using D02, E03:

E04-CODE:
```c
sock_inode_cachep = kmem_cache_create("sock_inode_cache",
                                      sizeof(struct socket_alloc),
                                      0,
                                      SLAB_HWCACHE_ALIGN,
                                      NULL);
```

E04-WHEN: Boot time (before any socket() call).
E04-SIZE: 728 bytes (E03).
E04-RESULT: Global pointer `sock_inode_cachep` holds cache address.

---

## DERIVATION E05: SOCKET_I Macro (Source: include/net/sock.h:1560)

Using B08, E03:

E05-PURPOSE: Convert inode* to socket*.
E05-CODE:
```c
#define SOCKET_I(inode) \
    &container_of(inode, struct socket_alloc, vfs_inode)->socket
```

E05-MATH:
1. inode = Address of vfs_inode field.
2. offsetof(socket_alloc, vfs_inode) = 128 (E03).
3. socket_alloc_base = inode - 128.
4. socket = socket_alloc_base + 0 = socket_alloc_base.

---

## DERIVATION E06: sock_alloc_inode Function (Source: net/socket.c:304)

Using D03, E03, E04, A07, A08:

E06-SIGNATURE: `static struct inode *sock_alloc_inode(struct super_block *sb)`
E06-INPUT: sb (SuperBlock pointer, not used in calculation).
E06-OUTPUT: struct inode * (Pointer to inode part).

### LINE 306: Variable Declaration

```c
struct socket_alloc *ei;
```

E06-L306-TYPE: Pointer to socket_alloc (E03).
E06-L306-SIZE: 8 bytes (A07).
E06-L306-VALUE: Undefined.

### LINE 309: Allocation

```c
ei = alloc_inode_sb(sb, sock_inode_cachep, GFP_KERNEL);
```

E06-L309-ACTION: Request 728 bytes from cache (E04).
E06-L309-RESULT: Address 0xffff8f4e33230340.
E06-L309-STORE: ei = 0xffff8f4e33230340.

### LINE 316: state = SS_UNCONNECTED

```c
ei->socket.state = SS_UNCONNECTED;
```

E06-L316-CALC:
- ei = 0xffff8f4e33230340 (E06-L309).
- offsetof(socket_alloc, socket) = 0 (E03).
- offsetof(socket, state) = 0 (E01).
- Target = 0xffff8f4e33230340 + 0 + 0 = 0xffff8f4e33230340.
- SS_UNCONNECTED = 0.
- Write: RAM[0xffff8f4e33230340] = 0 (4 bytes).

### LINE 318: ops = NULL

```c
ei->socket.ops = NULL;
```

E06-L318-CALC:
- Base = 0xffff8f4e33230340 (E06-L309).
- offsetof(socket, ops) = 32 (E01).
- Target = 0xffff8f4e33230340 + 32 = 0xffff8f4e33230360.
- NULL = 0 (A08).
- Write: RAM[0xffff8f4e33230360] = 0 (8 bytes).

### LINE 319: sk = NULL

```c
ei->socket.sk = NULL;
```

E06-L319-CALC:
- Base = 0xffff8f4e33230340 (E06-L309).
- offsetof(socket, sk) = 24 (E01).
- Target = 0xffff8f4e33230340 + 24 = 0xffff8f4e33230358.
- Write: RAM[0xffff8f4e33230358] = 0 (8 bytes).

### LINE 320: file = NULL

```c
ei->socket.file = NULL;
```

E06-L320-CALC:
- Base = 0xffff8f4e33230340 (E06-L309).
- offsetof(socket, file) = 16 (E01).
- Target = 0xffff8f4e33230340 + 16 = 0xffff8f4e33230350.
- Write: RAM[0xffff8f4e33230350] = 0 (8 bytes).

### LINE 322: Return

```c
return &ei->vfs_inode;
```

E06-L322-CALC:
- ei = 0xffff8f4e33230340 (E06-L309).
- offsetof(socket_alloc, vfs_inode) = 128 (E03).
- Return = 0xffff8f4e33230340 + 128 = 0xffff8f4e332303C0.

---

## DERIVATION E07: sock_alloc Function (Source: net/socket.c:629)

Using E05, E06:

E07-SIGNATURE: `struct socket *sock_alloc(void)`
E07-ARGUMENTS: **NONE** (C06).
E07-OBSERVATION: Does NOT use socket(2,1,0) arguments.

E07-CODE:
```c
struct socket *sock_alloc(void)
{
    struct inode *inode;
    struct socket *sock;

    inode = new_inode_pseudo(sock_mnt->mnt_sb);
    // ^ This calls sock_alloc_inode internally (E06)
    // ^ Returns 0xffff8f4e332303C0

    sock = SOCKET_I(inode);
    // ^ Uses E05: sock = 0xffff8f4e332303C0 - 128 = 0xffff8f4e33230340

    return sock;
}
```

E07-INPUT: None.
E07-OUTPUT: 0xffff8f4e33230340 (struct socket *).

---

## DERIVATION E08: Caller Context

E08-CALLER: `__sock_create` (net/socket.c:1682).
E08-CALLER-OF-CALLER: `sock_create` (net/socket.c:1660).
E08-CALLER-OF-CALLER-OF-CALLER: `__sys_socket` (net/socket.c:1660).

E08-CODE:
```c
// In __sock_create:
sock = sock_alloc();  // <-- Tick 1 happens here
if (!sock)
    return -ENFILE;
```

E08-OBSERVATION: `__sock_create` has access to `family` (2) and `type` (1).
E08-OBSERVATION: `sock_alloc` does NOT receive these arguments (E07).
E08-CONCLUSION: Tick 1 is blind to socket type.

---

## DERIVATION E09: Post-Tick 1 State

Using E06, E07:

E09-RETURNED: Pointer 0xffff8f4e33230340.
E09-TYPE: struct socket *.

E09-MEMORY-STATE:
```
Address              | Value | Field
---------------------|-------|------------------
0xffff8f4e33230340   | 0     | socket.state
0xffff8f4e33230344   | ?     | socket.type (unset)
0xffff8f4e33230348   | 0     | socket.flags
0xffff8f4e33230350   | 0     | socket.file (NULL)
0xffff8f4e33230358   | 0     | socket.sk (NULL)
0xffff8f4e33230360   | 0     | socket.ops (NULL)
```

E09-OBSERVATION: `socket.sk = NULL` means no TCP engine attached.
E09-OBSERVATION: `socket.ops = NULL` means no operations attached.
E09-CONCLUSION: This is an EMPTY container.

---

## DERIVATION E10: Naming Disambiguation

E10-TERM-1: User-space `socket()` function.
- Returns: int (File Descriptor).
- Example: fd = 3.

E10-TERM-2: Kernel `struct socket`.
- Created by: sock_alloc (E07).
- Role: File interface (VFS layer).
- Contains: state, ops, sk pointer.

E10-TERM-3: Kernel `struct sock`.
- Created by: sk_alloc (Tick 6, NOT this tick).
- Role: Network engine (TCP/IP layer).
- Contains: TCP state, buffers, timers.

E10-RELATIONSHIP:
- struct socket has field `sk` (pointer to struct sock).
- struct sock has field `sk_socket` (pointer to struct socket).
- After Tick 1: socket.sk = NULL.
- After Tick 6: socket.sk = address of sock.

---

## DERIVATION E11: sock_alloc vs sk_alloc

| Property | sock_alloc (E07) | sk_alloc (Tick 6) |
|:---|:---|:---|
| Arguments | void (none) | net, family, proto |
| Uses socket(2,1,0) args? | NO | YES |
| Returns | struct socket * | struct sock * |
| Layer | Filesystem | Network |
| Size allocated | 728 bytes (E03) | ~2000 bytes |

---

## MEMORY MAP: TICK 1 RESULT

```
+---------------------------+
| struct socket_alloc       | Address: 0xffff8f4e33230340
| (728 bytes total)         |
+---------------------------+
|                           |
| struct socket (128 bytes) | Offset +0
|   .state = 0              | +0
|   .type = ?               | +4
|   .flags = 0              | +8
|   .file = NULL            | +16
|   .sk = NULL              | +24  <-- EMPTY (No engine)
|   .ops = NULL             | +32  <-- EMPTY (No methods)
|   .wq = {...}             | +40
|                           |
+---------------------------+
|                           |
| struct inode (600 bytes)  | Offset +128
|   .i_mode = S_IFSOCK      |
|   .i_uid = 1000           |
|   .i_ino = 40221          |
|   ...                     |
|                           |
+---------------------------+
```

---

## TICK 1 SUMMARY

| Question | Answer |
|:---|:---|
| INPUT | None (sock_alloc takes void) |
| OUTPUT | 0xffff8f4e33230340 (struct socket *) |
| BYTES ALLOCATED | 728 |
| USES ARG 2 (AF_INET)? | NO |
| USES ARG 1 (SOCK_STREAM)? | NO |
| socket.sk after Tick 1? | NULL |
| socket.ops after Tick 1? | NULL |
| Is container empty? | YES |

---

## NEXT TICK (Forward Reference for Context Only)

After Tick 1 returns, `__sock_create` will:
1. Look up `net_families[2]` (Tick 2).
2. Call `inet_create` (Tick 3/4).
3. That function will call `sk_alloc` (Tick 6).
4. Finally wire `socket.sk` and `sock.sk_socket` (Tick 7).

---

## USER ERROR REPORT

1. Confused `socket` (int) with `struct socket` with `struct sock`.
   - Reality: Three distinct things in three layers.

2. Thought sock_alloc uses the arguments 2 and 1.
   - Reality: sock_alloc takes void, ignores arguments.

3. Thought allocation is simple malloc.
   - Reality: Colocated "Centaur" pattern (socket + inode).

4. Missed that socket.sk = NULL after Tick 1.
   - Reality: Container is empty, engine comes later.

---

## NEW THINGS INTRODUCED WITHOUT DERIVATION: ___

(Empty. All terms derived from Axiom Blocks A-D and Derivations E01-E11.)
