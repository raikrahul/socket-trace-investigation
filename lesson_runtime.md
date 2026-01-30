---
layout: default
title: "03_RUNTIME"
---

# 03. RUNTIME: THE SYSCALL

**Status**: Pending Trace Generation.

This document will analyze the execution path of `socket(AF_INET, SOCK_STREAM, 0)`.

## PLANNED TRACE

1.  **THE TRAP**: User Space (`syscall`) to Kernel Space (`entry_SYSCALL_64`).
2.  **THE ALLOCATION**: `sock_alloc()` interacting with the SLAB cache.
3.  **THE DISPATCH**: `inet_create()` resolution via `inetsw`.
4.  **THE RETURN**: File Descriptor derivation.

**[ BACK TO INDEX ](index.html)**
