# NUMERICAL RECONSTRUCTION: THE PRIMATE ARITHMETIC

## 01. THE PROBLEM (INPUT)
Input Values: `2`, `1`, `0`.
- `2` (AF_INET)
- `1` (SOCK_STREAM)
- `0` (IPPROTO_IP)

Goal: Convert `(2, 1, 0)` -> `File Descriptor (Integer)`.

## 02. ARITHMETIC STEP 1: RESOLVING THE ZERO
Input: `0`.
Problem: Kernel needs a real protocol ID (e.g., 6 for TCP).
Data Structure: `inetsw[1]` (Linked List).
Operation: Linear Scan.

**Calculation:**
1.  Read `inetsw[1]` Pointer.
2.  Read Node 1 (TCP):
    - `answer->protocol` = 6.
    - Check: `0 == 6`? FALSE.
    - Check: `0 == IPPROTO_IP (0)`? TRUE.
    - Result: Select 6.
    - **Output: 6.**

## 03. ARITHMETIC STEP 2: MEMORY ALLOCATION
Input: 768 (Requested Bytes).
Operation: `kmem_cache_alloc(sock_inode_cache)`.
Output: Block Address `A` (e.g., `0xffff888100000000`).

**Verification:**
- Valid Range: `[A, A + 832]` (Slab Size).
- **Output: Address A.**

## 04. ARITHMETIC STEP 3: THE POINTER SUBTRACTION
Input: Address `A` (The allocated block).
Problem: VFS expects an Inode Pointer. Network expects a Socket Pointer.
Constraint: Inode is at Offset 128.

**Calculation:**
1.  `Inode Pointer` = `A + 128`. (Returned to VFS).
2.  `Socket Pointer` = `Inode Pointer - 128`. (Recovered by Network).
3.  `A + 128 - 128 = A`.
4.  **Result: Socket Pointer == Block Base Address (A).**

## 05. ARITHMETIC STEP 4: BITMAP SCAN (FD)
Input: File Descriptor Table (Bitmap).
Operation: Find First Zero Bit.
Example State: `11100000` (Bits 0,1,2 used).
Calculation:
- Index 0: 1 (Used)
- Index 1: 1 (Used)
- Index 2: 1 (Used)
- Index 3: 0 (Free)
**Output: 3.**

## 06. THE FINAL EQUATION
`socket(2, 1, 0)` = `3`.
Side Effects:
- `inetsw` scanned (0 -> 6).
- `slab` allocated (832 bytes).
- `memory` mapping: `inode = socket + 128`.
