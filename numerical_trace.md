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
*   **Code Ref**: `net/socket.c:1696` (`pf = rcu_dereference(net_families[family]);`)
*   **Goal**: Find the factory for Family 2.
*   **Math**: `0xffffffff954767c0 + (2 * 8) = 0xffffffff954767d0`
*   **Action**: READ RAM at `0xffffffff954767d0`.
*   **Result**: `0xffffffff94183a20` (Pointer to `inet_family_ops`).

### TICK 3: THE METHOD LOOKUP (Finding the Factory)
*   **Code Ref**: `net/socket.c:1709` (`err = pf->create(...)`)
*   **Goal**: Find the "create" function pointer.
*   **Math**: `0xffffffff94183a20 + 8 = 0xffffffff94183a28`
*   **Action**: READ RAM at `0xffffffff94183a28`.
*   **Result**: `0xffffffff93d417b0` (Pointer to `inet_create`).

### TICK 4: THE JUMP (Entering the Factory)
*   **Action**: CALL `0xffffffff93d417b0` (`inet_create`).
*   **Code Ref**: `net/ipv4/af_inet.c:251` (`static int inet_create(...)`)

### TICK 5: THE TYPE LOOKUP (Using "1")
*   **Code Ref**: `net/ipv4/af_inet.c:271` (`list_for_each_entry(... &inetsw[sock->type] ...)`)
*   **Goal**: Find the TCP settings.
*   **Math**: `inetsw` base + `(1 * size)`.
*   **Result**: Pointer to `tcp_protosw` (`net/ipv4/af_inet.c:1153`).

### TICK 6: THE CONSTRUCTION (Building the Object)
*   **Code Ref**: `net/ipv4/af_inet.c:325` (`sk = sk_alloc(...)`)
    *   Allocates `0xffff8f4e2e47e880`.
    *   Passes `answer_prot` (TCP) and `PF_INET` (2).
*   **Code Ref**: `net/core/sock.c:2184` (`sk_alloc` implementation)
    *   Sets `sk->sk_family = family` (2).
    *   Sets `sk->sk_prot = prot` (TCP).

### TICK 7: THE WIRING
*   **Code Ref**: `net/core/sock.c:3387` (`sock_init_data(sock, sk)`)
    *   `sk->sk_socket = sock` (`0xffff8f4e33230340`)
    *   `sock->sk = sk` (`0xffff8f4e2e47e880`)

## THE RESULT
You hold File Descriptor 3.
FD 3 -> File -> Socket -> Sock (IPv4/TCP).
