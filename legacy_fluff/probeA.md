# PROBE 1: THE ENTRY POINT & KASLR PROOF

## 1. THE TARGET
*   **Function**: `__sys_socket`
*   **Source**: `net/socket.c`
*   **Role**: The first C function in the kernel that handles the socket system call logic.

## 2. THE ADDRESSES (Evidence of KASLR)

### A. LIVE (Runtime)
*   **Command**: `sudo grep -w "_text" /proc/kallsyms`
*   **Address**: `0xffffffff92a00000` (Kernel Start in RAM)
*   **Origin**: The actual memory location where the kernel is running RIGHT NOW.

### B. STATIC (Compile-time)
*   **Command**: `sudo grep -w "_text" /boot/System.map-$(uname -r)`
*   **Address**: `0xffffffff81000000` (Compiled Start)
*   **Origin**: The address assigned by the linker during compilation.

## 3. THE DIFFERENCE (Offset Calculation)
*   **Math**: `0x92a00000` - `0x81000000` = **`0x11a00000`**
*   **Verification**: 
    - `0x93bcfaf0` (__sys_socket LIVE) - `0x821cfaf0` (__sys_socket STATIC) = **`0x11a00000`**
*   **Conclusion**: The Kernel Randomly Slid itself by `0x11a00000` bytes (282 MB) at boot time.
*   **Why?**: Security (ASLR/KASLR) to prevent exploit targeting.

## 4. PROBING STRATEGY
*   We use **Symbol Name** (`"__sys_socket"`) for `kprobes`.
*   The Kernel automatically resolves `Symbol` -> `Live Address`.
*   This bypasses the need to manually calculate the offset.

## 5. REPRODUCIBILITY LOG
The following commands were run to verify this proof:
1. `uname -r` -> `6.14.0-37-generic`
2. `sudo apt-get install linux-image-$(uname -r)-dbgsym`
3. `sudo grep -w "__sys_socket" /proc/kallsyms`
4. `sudo gdb -batch -ex "print &__sys_socket" /usr/lib/debug/boot/vmlinux-...`
5. `sudo cat /boot/config-$(uname -r) | grep CONFIG_PHYSICAL_START`
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

## 10. LIVE EXECUTION VERIFICATION (Probe 1)
We compiled and ran the `socket_probe.c` module alongside `socket_test.c`:

```text
[ 3160.974378] [PROBE] Single-Probe Driver (__sys_socket) Loaded.
[ 3160.979220] [PROBE] __sys_socket ENTRY: family=2, type=1, protocol=0
```

*   **Family 2**: `AF_INET` (Verified)
*   **Type 1**: `SOCK_STREAM` (Verified)
*   **Protocol 0**: `Default` (Verified)

This proves that `socket_probe.c` successfully intercepted the exact system call made by `socket_test`.
