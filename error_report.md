# USER ERROR REPORT

## ERROR 1: ASSUMING COMPILER WRITES ADDRESSES
**User Thought:** "Compiler passes address 0xffffffff81a9b3c0."
**Reality:** Compiler writes *offsets/distances*.
**Orthogonal Thought:** If compiler wrote absolute addresses, KASLR would break instantly. Code must be position-independent or relocatable.
**Correction:** Compiler writes `RIP + DISTANCE`. Loader/CPU computes Address.

## ERROR 2: CONFUSING ENUM WITH SWITCH/CASE
**User Thought:** "Enum means switch case lookup."
**Reality:** Enums are integers. Arrays use integers as indices.
**Orthogonal Thought:** `switch(x)` compiles to jump tables or comparisons. `array[x]` compiles to `base + x*size`. `net_families[2]` is direct RAM access, not control flow logic.
**Correction:** `2` is just an offset multiplier (2 * 8 bytes).

## ERROR 3: MISSING THE LINKING STEP (BOOT VS RUNTIME)
**User Thought:** "How did pf get populated? Who put it there?"
**Reality:** Populated at *boot* by `sock_register`. Read at *runtime* by `socket()`.
**Orthogonal Thought:** Global state implies initialization time. If `socket()` reads X, X must exist before `socket()` runs. Therefore, X was created during kernel boot.
**Correction:** `sock_register` (Boot) -> `net_families[]` (Storage) -> `socket()` (Runtime).

## ERROR 4: IGNORING RIP-RELATIVE ADDRESSING
**User Thought:** "How does code find data if base changes?"
**Reality:** Distance between code and data in same binary is CONSTANT.
**Orthogonal Thought:** If A and B are on the same sheet of paper, moving the paper doesn't change the distance between A and B.
**Correction:** `Target = Current_IP + Constant_Distance`.

## ERROR 5: "MAGIC" VS MECHANISM
**User Thought:** "It just happens." (Magic)
**Reality:** Every byte has an origin.
**Orthogonal Thought:** If a value exists, an instruction wrote it. If an instruction wrote it, it calculated it. Trace back to the calculation.
**Correction:** Trace: Source -> Compiler -> Binary -> Loader -> RAM -> CPU -> Value.
