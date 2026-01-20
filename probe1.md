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
