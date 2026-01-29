# LESSON 4: PRIMATE ARITHMETIC (EASY)

## 01. THE ALLOCATION
We need 768 bytes.
Kernel gives us Block `A`.
`A` points to the start of memory.

## 02. THE OFFSET
`struct socket` is at offset 0.
`struct inode` is at offset 128.

## 03. THE VIEWS
Network View:
`socket = A`.

VFS View:
`inode = A + 128`.

## 04. THE CONVERSION
When VFS hands us an `inode`, we want the `socket`.
`socket = inode - 128`.
`(A + 128) - 128 = A`.

**Axiom:** Subtraction recovers the original handle.
