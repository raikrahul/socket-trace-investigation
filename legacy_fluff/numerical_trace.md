# THE NUMERICAL TRACE OF SOCKET(2, 1, 0)
## ANNOTATED WITH LINUX 6.8 KERNEL SOURCE

## THE PROBLEM
I am a user program. I execute the instruction `socket(2, 1, 0)`.
This creates a complex object in kernel memory.
**HOW?** How does the number `2` turn into an IPv4 object? How does `1` turn into TCP?

---

## CONTEXT: THE MAP (Verified on Machine)

1.  **THE ARRAY (The Directory)**
    *   **Code Ref**: `net/socket.c:123` (`net_families`)
    *   **Address**: `0xffffffff954767c0`

2.  **THE STRUCT (The IPv4 Blueprint)**
    *   **Code Ref**: `net/ipv4/af_inet.c:1142` (`inet_family_ops`)
    *   **Address**: `0xffffffff94183a20`

3.  **THE FUNCTION (The IPv4 Factory)**
    *   **Code Ref**: `net/ipv4/af_inet.c:251` (`inet_create`)
    *   **Address**: `0xffffffff93d417b0`

4.  **THE SUB-ARRAY (The TCP Lookup)**
    *   **Code Ref**: `net/ipv4/af_inet.c:128` (`inetsw`)
    *   **Verified Content**: Index 1 -> `tcp_protosw`

---

## THE EXECUTION TICK-TOCK

### TICK 1: THE CALL
*   **You**: `socket(2, 1, 0)`
*   **Kernel Entry**: `net/socket.c:1660` (`__sys_socket`)
*   **Action**: Call `sock_alloc()` (`net/socket.c:1682`)

    **SUB-TRACE: sock_alloc() (`net/socket.c:629`)**
    1.  `inode = new_inode_pseudo(sock_mnt->mnt_sb)`
        *   Allocates VFS inode (Filesystem layer).
    2.  `sock = SOCKET_I(inode)`
        *   **Math**: `sock` is part of `socket_alloc` struct.
        *   `container_of` math converts Inode Address -> Socket Address.
    3.  **Result**: Returns `sock = 0xffff8f4e33230340`
        *   This creates the **Container** (top half).
        *   It is currently empty (`sock->sk = NULL`).

### TICK 2: THE DIRECTORY LOOKUP (Using "2")
*   **Goal**: Find the factory for Family 2.
*   **Source**: `net/socket.c:1696` (`pf = rcu_dereference(net_families[family]);`)
*   **Logic**:
    1.  Kernel takes base address of `net_families` array (`0xffffffff954767c0`).
    2.  Calculates offset: `2 * 8 bytes = 16 bytes (0x10)`.
    3.  Adds offset: `...67c0 + 0x10 = ...67d0`.
    4.  **Action**: READ RAM at `0xffffffff954767d0`.
*   **Result**: Finds pointer `0xffffffff94183a20` (Address of `inet_family_ops`).

### TICK 3: THE METHOD LOOKUP (Finding the Factory)
*   **Goal**: Find the "create" function pointer.
*   **Source**: `net/socket.c:1709` (`err = pf->create(net, sock, protocol, kern);`)
*   **Logic**:
    1.  Kernel takes address of struct `inet_family_ops` (`0xffffffff94183a20`).
    2.  Check definition in `include/linux/net.h:212`:
        ```c
        struct net_proto_family {
            int family;        // 4 bytes
            // 4 bytes padding
            int (*create)(...); // Offset 8
        };
        ```
    3.  Adds offset: `...3a20 + 8 = ...3a28`.
    4.  **Action**: READ RAM at `0xffffffff94183a28`.
*   **Result**: Finds pointer `0xffffffff93d417b0` (Address of `inet_create`).

### TICK 4: THE JUMP (Entering the Factory)
*   **Action**: CALL `0xffffffff93d417b0`.
*   **Source**: `net/ipv4/af_inet.c:251` (`static int inet_create(...)`)
*   **Context Change**:
    *   Old Context: Generic Socket Layer (`net/socket.c`).
    *   New Context: IPv4 Specific Layer (`net/ipv4/af_inet.c`).
    *   Arguments passed: `sock` (Container), `1` (Protocol/Type).

### TICK 5: THE TYPE LOOKUP (Using "1")
*   **Goal**: Find the TCP settings.
*   **Source**: `net/ipv4/af_inet.c:271` (`list_for_each_entry_rcu(answer, &inetsw[sock->type], list)`)
*   **Logic**:
    1.  Kernel takes base of `inetsw` array (`net/ipv4/af_inet.c:128`).
    2.  Uses argument `sock->type` (which is `SOCK_STREAM` = 1).
    3.  Accesses list head at `inetsw[1]`.
    4.  Iterates list (usually only one item: TCP).
*   **Result**: Finds pointer to `tcp_protosw` struct (Defined in `net/ipv4/af_inet.c:1153`).
    *   Content: `.protocol = IPPROTO_TCP` (6).
    *   Content: `.prot = &tcp_prot`.

### TICK 6: THE CONSTRUCTION (Building the Object)
*   **Goal**: Allocate the network-specific "Engine".
*   **Source**: `net/ipv4/af_inet.c:325` (`sk = sk_alloc(net, PF_INET, GFP_KERNEL, answer_prot, kern);`)
*   **SUB-TRACE: sk_alloc() (`net/core/sock.c:2128`)**
    1.  Allocates memory from generic slab.
    2.  `sk->sk_family = family;` (Line 2184) -> Writes `2` into the new struct.
    3.  `sk->sk_prot = prot;` (Line 2185) -> Writes `address of tcp_prot` into the new struct.
*   **Result**: Returns `sk = 0xffff8f4e2e47e880`.

### TICK 7: THE WIRING (Connecting Container & Engine)
*   **Goal**: Link the `socket` (Tick 1) and `sock` (Tick 6).
*   **Source**: `net/core/sock.c:3387` (`sock_init_data(sock, sk)`) called from `inet_create`.
*   **Logic**:
    1.  `sk->sk_socket = sock;`
        *   Engine points to Container.
        *   Writes `0xffff8f4e33230340` into `sk` struct.
    2.  `sock->sk = sk;`
        *   Container points to Engine.
        *   Writes `0xffff8f4e2e47e880` into `socket` struct.
*   **Final State**: The object is fully linked and initialized.
