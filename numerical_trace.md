# THE NUMERICAL TRACE OF SOCKET(2, 1, 0)

## THE PROBLEM
I am a user program. I execute the instruction `socket(2, 1, 0)`.
This creates a complex object in kernel memory.
**HOW?** How does the number `2` turn into an IPv4 object? How does `1` turn into TCP?
We must trace every pointer, every calculation, every jump using ONLY numbers found on your machine.

---

## CONTEXT: THE MAP (Verified on Machine)
These are the fixed locations in the kernel (The "Board"):

1.  **THE ARRAY (The Directory)**
    *   Address: `0xffffffff954767c0`
    *   Name: `net_families`
    *   Verified Content at Index 2: `0xffffffff94183a20`

2.  **THE STRUCT (The IPv4 Blueprint)**
    *   Address: `0xffffffff94183a20`
    *   Name: `inet_family_ops`
    *   Verified Content at Offset 8: `0xffffffff93d417b0`

3.  **THE FUNCTION (The IPv4 Factory)**
    *   Address: `0xffffffff93d417b0`
    *   Name: `inet_create`

4.  **THE SUB-ARRAY (The TCP Lookup)**
    *   Name: `inetsw`
    *   Verified Content at Index 1: Pointer to `tcp_protosw`

---

## THE EXECUTION TICK-TOCK

### TICK 1: THE CALL
*   **You**: `socket(2, 1, 0)`
*   **CPU**: Jumps to kernel `__sys_socket`.
*   **Registers**: `RDI=2`, `RSI=1`.

### TICK 2: THE DIRECTORY LOOKUP (Using "2")
*   **Goal**: Find the factory for Family 2.
*   **Math**: `Array_Address + (Index * 8)`
    *   `0xffffffff954767c0` + `(2 * 8)` = `0xffffffff954767d0`
*   **Action**: READ RAM at `0xffffffff954767d0`.
*   **Result**: `0xffffffff94183a20` (This is the address of the IPv4 Blueprint).

### TICK 3: THE METHOD LOOKUP (Finding the Factory)
*   **Goal**: Find the "create" function inside the blueprint.
*   **Math**: `Struct_Address + Offset_of_Create`
    *   `0xffffffff94183a20` + `8` = `0xffffffff94183a28`
*   **Action**: READ RAM at `0xffffffff94183a28`.
*   **Result**: `0xffffffff93d417b0` (This is the address of `inet_create`).

### TICK 4: THE JUMP (Entering the Factory)
*   **Action**: CALL `0xffffffff93d417b0` with arguments `(sock, 1)`.
*   **Status**: We are now inside `inet_create`. The kernel knows it is building IPv4 because it is running IPv4 code.

### TICK 5: THE TYPE LOOKUP (Using "1")
*   **Goal**: Find the TCP settings.
*   **Context**: We are inside `inet_create`.
*   **Math**: `SubArray_inetsw + (Type * Size)`
    *   `inetsw` + `(1 * Size)`
*   **Action**: READ RAM at that slot.
*   **Result**: Pointer to `tcp_protosw`.

### TICK 6: THE CONSTRUCTION (Building the Object)
*   **Action**: Allocate memory for `struct sock` (Address `0xffff8f4e2e47e880`).
*   **Action**: Copy settings from `tcp_protosw` into the new `struct sock`.
    *   `sk->sk_prot = tcp_prot` (Sets up TCP function table).
    *   `sk->sk_family = 2` (Writes the number 2 into the memory).

### TICK 7: THE WIRING
*   **Context**: We have `socket` (Container) and `sock` (Engine).
*   **Action**: Link them.
    *   `socket->sk = 0xffff8f4e2e47e880`
    *   `sock->sk_socket = 0xffff8f4e33230340`

## THE RESULT
You hold File Descriptor 3.
FD 3 -> File -> Socket -> Sock (IPv4/TCP).

Every jump was a calculated memory read.
No magic. Just offsets and pointers.
