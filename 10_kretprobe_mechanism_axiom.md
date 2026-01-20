# Axiomatic Derivation: The Mechanism of Kretprobe

## 1. The Physical Reality (Axioms)

01. **CPU Registers**: The "Hands" of the CPU. Fast, temporary storage.
    *   **RSP (Stack Pointer)**: A specific register that holds a *Number*. This number is an *Address* pointing to a location in RAM.
    *   **RIP (Instruction Pointer)**: A specific register that holds the Address of the *current* instruction attempting to execute.

02. **RAM (Random Access Memory)**: The "Notebook". Massive grid of storage bytes.
    *   **The Stack**: A reserved region in RAM.
    *   **The Text Segment**: A Read-Only region in RAM where code lives.

03. **Opcode `CALL`**: When CPU executes `CALL function`:
    1.  Calculates the Address of the *next* instruction (Result: `0xRET_ADDR`).
    2.  Writes `0xRET_ADDR` to the RAM location pointed to by `RSP`.
    3.  Decrements `RSP` (Stack Grows Down).
    4.  Updates `RIP` to the address of `function`.

04. **Opcode `RET`**: When CPU executes `RET`:
    1.  Increments `RSP`.
    2.  Reads the value from RAM at the location pointed to by `RSP`.
    3.  Updates `RIP` with that value (Jumps back).

---

## 2. The Setup (Before The Trap)

**Scenario**: User calls `socket()`.
*   User Code Address: `0x100` calling function at `0x1000`.
*   Return Address: `0x105` (Instruction after call).
*   Stack Pointer (`RSP`): `0x9000`.

**State at T=0 (Just before execution)**:
*   `RIP`: `0x100`
*   `RSP`: `0x9008`
*   RAM[`0x9000`]: [Empty/Zero]

**State at T=1 (After CALL executes)**:
*   `RIP`: `0x1000` (Start of `__sys_socket`)
*   `RSP`: `0x9000`
*   RAM[`0x9000`]: **0x105** (The Real Return Address)

---

## 3. The Trap (Context Switch)

**The Intervention**: We registered a `kretprobe` at `__sys_socket` (`0x1000`).
The Kernel has physically overwritten the byte at `0x1000` with `INT 3` (Breakpoint).

**State at T=2 (CPU hits 0x1000)**:
1.  **Exception**: CPU encounters `INT 3`.
2.  **Context Save**: CPU saves current registers (including `RSP=0x9000`) to a struct `pt_regs` in Kernel Memory.
3.  **Jump**: CPU jumps to Kernel Exception Handler (`do_int3`).

---

## 4. The Hijack (Inside The Handler)

**Kernel Execution**:
1.  Kernel sees the Trap was caused by our `kretprobe`.
2.  Kernel calls `pre_handler_kretprobe`.
3.  **Crucial Step 1 (Allocation)**: Kernel allocates an instance `ri` (Backpack).
4.  **Crucial Step 2 (Reading History)**:
    *   Kernel looks at saved `pt_regs->rsp` (`0x9000`).
    *   Kernel reads RAM at `0x9000`.
    *   Value Found: **0x105**.
5.  **Crucial Step 3 (Preservation)**:
    *   Kernel saves `0x105` into `ri->ret_addr`.
6.  **Crucial Step 4 (The Hack)**:
    *   Kernel **OVERWRITES** RAM at `0x9000`.
    *   New Value: **0xTRAMPOLINE** (Address of `kretprobe_trampoline`).

**State at T=3 (RAM Modified)**:
*   RAM[`0x9000`]: **0xTRAMPOLINE** (Previously 0x105).

---

## 5. The Execution (Back to Normal?)

**Kernel Execution**:
1.  Kernel restores registers from `pt_regs`.
2.  Kernel executes the *Original Instruction* (from `0x1000`) in a buffer.
3.  Kernel jumps back to `0x1001` (Instruction 2 of function).

**State at T=4 (Function Running)**:
*   `__sys_socket` executes normally.
*   It calculates return values...
*   It finishes.

---

## 6. The Return (The Trap Triggered)

**State at T=5 (Function executes RET)**:
1.  CPU reads Stack at `0x9000`.
2.  CPU expects `0x105`.
3.  CPU finds **0xTRAMPOLINE**.
4.  CPU updates `RIP` to `0xTRAMPOLINE`.

**State at T=6 (Inside Trampoline)**:
1.  Trampoline Code pauses execution.
2.  Finds the backpack `ri`.
3.  Calls our `ret_handler` (The Exit Probe).
    *   We verify args, calculate latency, check `regs->ax`.
4.  Trampoline reads `ri->ret_addr` (`0x105`).
5.  Trampoline jumps to `0x105`.

**Result**: User Space resumes, oblivious to the fact it just took a detour through our logging logic.
