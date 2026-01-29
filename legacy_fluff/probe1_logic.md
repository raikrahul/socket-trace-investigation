# AXIOMATIC EXPLANATION: KERNEL MEMORY AND KASLR

## 01. THE STARTING POINT (Physical Reality)
*   **Axiom 1**: Computers have RAM (Random Access Memory).
*   **Axiom 2**: RAM is a linear array of bytes, numbered `0` to `MEM_MAX`.
*   **Axiom 3**: The CPU executes instructions stored in these bytes.

## 02. THE PHYSICAL ADDRESS (The Hardware View)
*   **Definition**: The number printed on the "mailbox" of a RAM byte.
*   **Example**: `0x1000000` is the 16,777,216th byte in your RAM stick.
*   **Fact**: The Kernel (the OS Core) is loaded into RAM at boot.
*   **Config**: `CONFIG_PHYSICAL_START` tells the bootloader to load the kernel at `0x1000000` (16MB mark).

## 03. THE VIRTUAL ADDRESS (The Software View)
*   **Axiom 4**: The CPU has a "Translator" called the MMU (Memory Management Unit).
*   **Function**: Software uses Fake Addresses (Virtual) -> MMU -> Real Addresses (Physical).
*   **Mapping**: The Kernel decides to live at the "Top" of the address space (`0xffffffff...`).
*   **Static Mapping**: The compiler (`gcc`/linker) assumes the kernel starts at `0xffffffff81000000`.
    - `0xffffffff80000000` + `0x1000000` (Physical Start) = `0xffffffff81000000` (approx logic).

## 04. THE SECURITY PROBLEM
*   **Fact**: If the kernel ALWAYS starts at `...81000000`, hackers know exactly where functions are.
*   **Attack**: "Jump to `...81000000` + 50 to call `root_access()`".
*   **Solution**: Move the kernel to a random spot every time it boots.

## 05. KASLR (Kernel Address Space Layout Randomization)
*   **Action**: At boot, the generic loader picks a random number (`OFFSET`).
*   **New Mapping**: `Virtual_Addr = Static_Addr + OFFSET`.

## 06. THE EVIDENCE (Your System)
*   **Step A (Static)**: We checked `System.map`.
    - Compiler said: "I start at `0xffffffff81000000`".
*   **Step B (Live)**: We checked `/proc/kallsyms` (The MMU's current truth).
    - CPU said (MMU): "I am actually at `0xffffffff92a00000`".
*   **Step C (Derivation)**:
    - `92a00000` - `81000000` = `11a00000`.
    - **∴ OFFSET = `0x11a00000`**.

## 07. RELEVANCE TO PROBING
*   **Problem**: If you tell `kprobe` to spy on `0xffffffff81000000`, you spy on EMPTY SPACE (Crash/Silence).
*   **Correction**: You must add the `OFFSET` (`0x11a00000`) to find the REAL code.
*   **Automation**: The `kprobe` system inside the kernel knows its own offset, so we just give it the NAME (`_text` or `__sys_socket`) and it handles the math.

## 08. FINAL EQUATION
*   `Live_Addr(__sys_socket)` // `0xffffffff93bcfaf0`
*   **EQUALS** (=)
*   `Static_Addr(__sys_socket)` // `0xffffffff821cfaf0`
*   **PLUS** (+)
*   `Run_Time_Offset` // `0x11a00000`

**(Mathematics Verified by GDB and Kallsyms)**

## 09. RAW EVIDENCE (System.map)
The following are the static addresses extracted directly from the disk file `/boot/System.map-6.14.0-37-generic`:

```text
ffffffff821cfaf0 T __sys_socket
ffffffff81000000 T _text
```
*(These match the "Static" values used in the equation above.)*

