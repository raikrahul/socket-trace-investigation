# LESSON 7: THE REGISTERS (ABI TRUTH)

## 01. THE ABI CONTRACT
System V AMD64 ABI dictates register usage for function calls.
- Arg 1: `%rdi`
- Arg 2: `%rsi`
- Arg 3: `%rdx`

## 02. THE MAPPING
Input: `socket(2, 1, 0)`

Instruction Stream (Assembly):
1. `mov $2, %rdi`  (Family)
2. `mov $1, %rsi`  (Type)
3. `mov $0, %rdx`  (Protocol)

## 03. THE INTERFACE
The C Wrapper (`socket()`) prepares these registers to match the Kernel's expectation.

**Axiom:** The CPU does not know "Family" or "Type". It only knows values in `RDI` and `RSI`.
