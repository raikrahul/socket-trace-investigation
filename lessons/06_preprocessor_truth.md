# LESSON 6: THE PREPROCESSOR (GAPLESS TRUTH)

## 01. THE INPUT
Code: `socket(AF_INET, SOCK_STREAM, 0);`

## 02. THE HIDDEN SUBSTITUTION
Before compilation, the Preprocessor replaces text.
Source: `/usr/include/bits/socket.h` (or similar).
- `#define AF_INET 2`
- `#define SOCK_STREAM 1`

## 03. THE OUTPUT (PURE C)
The Compiler sees:
`socket(2, 1, 0);`

**Axiom:** `AF_INET` does not exist in the binary. Only the integer `2` remains.
