# PROOF OF THE "MAGIC" VALUES

## PART 1: WHY IS .create AT OFFSET 8? (Axiom: C Struct Layout)

It is not magic. It is defined by the C standards and the x64 Architecture.

### Axiom 1: Data Sizes on x64
1.  `int` = 4 bytes
2.  `pointer` = 8 bytes
3.  `alignment` = A pointer must usually start at an address divisible by 8.

### Axiom 2: The Struct Definition
The kernel code says:
```c
struct net_proto_family {
    int family;        // Field 1
    int (*create)(...); // Field 2 (Function Pointer)
};
```

### Derivation of Memory Layout
1.  **Offset 0**: `family` takes 4 bytes. (Bytes 0, 1, 2, 3 occupied).
2.  **Next Free Byte**: Byte 4.
3.  **Constraint**: The next field is a POINTER.
4.  **Alignment Rule**: On x64, an 8-byte pointer must start at an offset divisible by 8.
5.  **Padding**: The compiler inserts 4 bytes of "padding" (garbage) at Bytes 4, 5, 6, 7.
6.  **Offset 8**: Now we are aligned. The `create` pointer goes here. (Bytes 8-15).

**CONCLUSION:**
The instruction "Call the function at offset 8" is baked into the kernel binary by the compiler because it KNOWS the layout rule.

---

## PART 2: THE BOOT TIME INITIALIZATION (Axiom: Assignment)

You asked: *"This value was put there by sock_register at boot time... this bothers me"*

### Axiom 3: Global Variables start at 0
When the kernel loads, the `net_families` array is all zeros. `NULL`.

### Axiom 4: Assignment is Copying
The code `net_families[2] = &inet_family_ops` runs at boot.
This copies the **Address** of the struct into the Array slot.

**Visualizing Boot Time:**

1.  **Before `af_inet_init` runs:**
    `net_families[2]` = `0x0000000000000000` (NULL)

2.  **The Code runs:**
    `sock_register` asks: "Where is `inet_family_ops`?"
    Answer: "It is at `0xffffffff94183a20`".

3.  **The Write:**
    CPU writes `0xffffffff94183a20` into the array slot.

4.  **Result:**
    Now the array acts as a signpost pointing to the struct.

---

## PART 3: THE COMPILE TIME INITIALIZATION (Axiom: Constant Data)

You asked: *"This value [function address] was put there by the compiler/loader... how?"*

### Axiom 5: Functions have Addresses
The code for `inet_create` exists. It has to live *somewhere* in memory. Let's say `0xffffffff93d417b0`.

### Axiom 6: Static Initalization
The code says:
```c
static const struct net_proto_family inet_family_ops = {
    .create = inet_create,
};
```

This tells the compiler: "When you build the binary file, DO NOT leave this struct empty. Go find where you put `inet_create`, calculate its address, and WRITE THAT NUMBER directly into the struct's byte at offset 8."

**Visualizing the Binary File on Disk:**
At the spot reserved for `inet_family_ops`:
*   Bytes 0-3: `02 00 00 00` (Number 2)
*   Bytes 4-7: `00 00 00 00` (Padding)
*   Bytes 8-15: `b0 17 d4 93 ff ff ff ff` (The address of the function!)

**CONCLUSION:**
The compiler hardcoded the function's address into the struct.
The boot code hardcoded the struct's address into the array.
So when you read the array, then the struct, you find the function.
