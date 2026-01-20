# TICK 1: THE VFS-SOCKET DANCE (Complete)

## THE PROBLEM

You call:
```c
int fd = socket(2, 1, 0);
```

The kernel must allocate memory. But sockets are also files.
Files are managed by VFS (Virtual File System).
So socket code must cooperate with VFS.

---

## WHO WROTE WHAT?

| File | Authors | Purpose |
|:---|:---|:---|
| fs/inode.c | Linus, VFS developers | Generic file/inode management |
| net/socket.c | Network developers | Socket-specific code |
| include/linux/fs.h | VFS developers | VFS structures and functions |
| include/linux/net.h | Network developers | Socket structures |

**KEY INSIGHT:**
VFS was written FIRST (1991).
Socket code was written to FIT INTO VFS.
VFS does not know about sockets.
Socket code "pretends" to be a filesystem.

---

## THE TWO STRUCTS

### struct socket (Network Layer)
```c
// include/linux/net.h:117
struct socket {
    socket_state state;           // Offset +0
    short        type;            // Offset +4
    unsigned long flags;          // Offset +8
    struct file *file;            // Offset +16
    struct sock *sk;              // Offset +24  <-- TCP engine (NULL now)
    const struct proto_ops *ops;  // Offset +32  <-- Methods (NULL now)
    struct socket_wq wq;          // Offset +40
};
// sizeof = 128 bytes
```

### struct inode (VFS Layer)
```c
// include/linux/fs.h:642
struct inode {
    umode_t i_mode;       // File type (socket, regular, etc.)
    kuid_t i_uid;         // Owner user ID
    kgid_t i_gid;         // Owner group ID
    unsigned long i_ino;  // Inode number
    // ... many more fields
};
// sizeof ~ 600 bytes
```

### struct socket_alloc (The Centaur)
```c
// include/net/sock.h:1555
struct socket_alloc {
    struct socket socket;   // Offset +0   (128 bytes)
    struct inode vfs_inode; // Offset +128 (600 bytes)
};
// sizeof = 728 bytes
```

---

## WHY THE CENTAUR PATTERN?

**Option 1: Two separate allocations**
```c
socket = kmalloc(128);
inode = kmalloc(600);
socket->inode = inode;  // Link them
```
Cost: 2 mallocs, 2 frees, bad cache locality.

**Option 2: One allocation (Centaur)**
```c
socket_alloc = kmalloc(728);  // Both in one block
```
Cost: 1 malloc, 1 free, good cache locality.

**Linux chose Option 2.**

---

## THE BOOT-TIME SETUP

At boot, kernel creates a memory pool for 728-byte objects:

```c
// net/socket.c:341
sock_inode_cachep = kmem_cache_create("sock_inode_cache",
                                      sizeof(struct socket_alloc),  // 728
                                      0,
                                      SLAB_HWCACHE_ALIGN,
                                      NULL);
```

Also registers the fake filesystem "sockfs":

```c
// net/socket.c:352
static const struct super_operations sockfs_ops = {
    .alloc_inode = sock_alloc_inode,  // <-- VFS will call THIS
    .free_inode  = sock_free_inode,
};
```

---

## THE RUNTIME CALL CHAIN

### Step 1: Your code calls socket()
```c
// User space
int fd = socket(2, 1, 0);
```

### Step 2: Kernel enters __sock_create
```c
// net/socket.c:1682
int __sock_create(struct net *net, int family, int type, int protocol,
                  struct socket **res, int kern)
{
    struct socket *sock;
    
    sock = sock_alloc();  // <-- Step 3
    ...
}
```

### Step 3: sock_alloc calls VFS
```c
// net/socket.c:629
struct socket *sock_alloc(void)
{
    struct inode *inode;
    struct socket *sock;

    inode = new_inode_pseudo(sock_mnt->mnt_sb);  // <-- Step 4 (into VFS)
    
    sock = SOCKET_I(inode);  // <-- Step 8 (subtract 128)
    
    inode->i_ino = get_next_ino();
    inode->i_mode = S_IFSOCK | S_IRWXUGO;
    inode->i_uid = current_fsuid();
    inode->i_gid = current_fsgid();
    inode->i_op = &sockfs_inode_ops;

    return sock;
}
```

### Step 4: VFS calls alloc_inode
```c
// fs/inode.c (VFS code)
struct inode *new_inode_pseudo(struct super_block *sb)
{
    struct inode *inode = alloc_inode(sb);  // <-- Step 5
    ...
    return inode;
}
```

### Step 5: VFS uses function pointer
```c
// fs/inode.c (VFS code)
static struct inode *alloc_inode(struct super_block *sb)
{
    struct inode *inode;

    if (sb->s_op->alloc_inode)
        inode = sb->s_op->alloc_inode(sb);  // <-- Step 6 (CALLBACK!)
    else
        inode = kmem_cache_alloc(inode_cachep, GFP_KERNEL);
    
    return inode;
}
```

For sockfs, `sb->s_op->alloc_inode` points to `sock_alloc_inode`.
VFS does NOT know about sockets. It just calls the function pointer.

### Step 6: OUR function allocates the Centaur
```c
// net/socket.c:304
static struct inode *sock_alloc_inode(struct super_block *sb)
{
    struct socket_alloc *ei;

    ei = alloc_inode_sb(sb, sock_inode_cachep, GFP_KERNEL);
    // ei = 0xffff8f4e33230340 (728 bytes allocated)
    
    if (!ei)
        return NULL;
    
    // Initialize wait queue
    init_waitqueue_head(&ei->socket.wq.wait);
    ei->socket.wq.fasync_list = NULL;
    ei->socket.wq.flags = 0;

    // Initialize socket fields to EMPTY
    ei->socket.state = SS_UNCONNECTED;  // = 0
    ei->socket.flags = 0;
    ei->socket.ops = NULL;   // NO METHODS
    ei->socket.sk = NULL;    // NO TCP ENGINE
    ei->socket.file = NULL;

    return &ei->vfs_inode;  // <-- Step 7 (return inode pointer)
    // Returns: 0xffff8f4e33230340 + 128 = 0xffff8f4e332303C0
}
```

### Step 7: Return flows back up
```
sock_alloc_inode returns  0xffff8f4e332303C0 (inode ptr)
         |
         v
alloc_inode receives      0xffff8f4e332303C0
         |
         v
new_inode_pseudo receives 0xffff8f4e332303C0
         |
         v
sock_alloc receives       0xffff8f4e332303C0 in "inode"
```

### Step 8: sock_alloc subtracts 128
```c
// Back in sock_alloc (net/socket.c:639)
sock = SOCKET_I(inode);
// SOCKET_I does: inode - 128 = 0xffff8f4e33230340
```

The SOCKET_I macro:
```c
// include/net/sock.h:1560
#define SOCKET_I(inode) \
    &container_of(inode, struct socket_alloc, vfs_inode)->socket
```

Math:
```
inode address:        0xffff8f4e332303C0
offset of vfs_inode:  128 (0x80)
socket_alloc base:    0xffff8f4e332303C0 - 0x80 = 0xffff8f4e33230340
socket offset:        0
socket address:       0xffff8f4e33230340
```

---

## THE COMPLETE TIMELINE

| Time | Function | File | Action | Address |
|:---|:---|:---|:---|:---|
| T1 | socket() | libc | System call | - |
| T2 | __sys_socket | net/socket.c | Kernel entry | - |
| T3 | __sock_create | net/socket.c | Calls sock_alloc | - |
| T4 | sock_alloc | net/socket.c | Calls new_inode_pseudo | - |
| T5 | new_inode_pseudo | fs/inode.c | VFS, calls alloc_inode | - |
| T6 | alloc_inode | fs/inode.c | VFS, calls function pointer | - |
| T7 | sock_alloc_inode | net/socket.c | Allocates 728 bytes | 0x...0340 |
| T8 | sock_alloc_inode | net/socket.c | Returns inode pointer | 0x...03C0 |
| T9 | alloc_inode | fs/inode.c | Returns to VFS | 0x...03C0 |
| T10 | new_inode_pseudo | fs/inode.c | Returns to sock_alloc | 0x...03C0 |
| T11 | sock_alloc | net/socket.c | SOCKET_I subtracts 128 | 0x...0340 |
| T12 | sock_alloc | net/socket.c | Returns socket pointer | 0x...0340 |

---

## MEMORY STATE AFTER TICK 1

```
Address              | Offset | Field           | Value
---------------------|--------|-----------------|--------
0xffff8f4e33230340   | +0     | socket.state    | 0
0xffff8f4e33230344   | +4     | socket.type     | ?
0xffff8f4e33230348   | +8     | socket.flags    | 0
0xffff8f4e33230350   | +16    | socket.file     | NULL
0xffff8f4e33230358   | +24    | socket.sk       | NULL (NO ENGINE)
0xffff8f4e33230360   | +32    | socket.ops      | NULL (NO METHODS)
0xffff8f4e33230368   | +40    | socket.wq       | initialized
...                  |        |                 |
0xffff8f4e332303C0   | +128   | inode.i_mode    | S_IFSOCK
0xffff8f4e332303C4   | +132   | inode.i_uid     | 1000
...                  |        |                 |
```

---

## WHY RETURN INODE, NOT SOCKET?

VFS code expects `struct inode *`.
VFS will write to inode fields:
```c
// After alloc_inode returns, VFS does:
inode->i_sb = sb;
inode->i_blkbits = sb->s_blocksize_bits;
// ... more inode initialization
```

If we returned socket pointer, VFS would write to WRONG memory.
VFS would overwrite socket.state, socket.type, etc.
**CRASH.**

So we return what VFS expects (inode pointer).
Then we secretly know socket is 128 bytes before it.

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

## WHAT HAPPENS NEXT (TICK 2+)

After sock_alloc returns, __sock_create will:
```c
// net/socket.c:1696
pf = rcu_dereference(net_families[family]);  // Tick 2: Lookup family 2
...
err = pf->create(net, sock, protocol, kern);  // Tick 3: Call inet_create
```

This is when the kernel finally uses the numbers 2 and 1.
