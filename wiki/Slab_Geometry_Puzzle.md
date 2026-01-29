# THE SLAB GEOMETRY PUZZLE: 768 vs 832

## 01. THE AXIOM
Kernel requests a cache for `struct socket_alloc`.
Size requested: `sizeof(struct socket_alloc)`.

## 02. THE PROBE DATA (Kernel 6.14.0-37)
- `sizeof(struct socket)` = 128 bytes
- `sizeof(struct inode)` = 624 bytes
- `sum` = 752 bytes
- `sizeof(struct socket_alloc)` = 768 bytes (Aligned to 64B cache line)
  - 752 bytes payload
  - 16 bytes internal padding
  - Total: 12 Cache Lines (12 * 64 = 768)

## 03. THE REALITY (slabinfo)
`$ cat /proc/slabinfo | grep sock_inode_cache`
`sock_inode_cache ... 832 ...`

## 04. THE DISCREPANCY
Requested: 768 bytes.
Allocated: 832 bytes.
Delta: 64 bytes (Exactly 1 Cache Line).

## 05. AXIOMATIC CONCLUSION
The Slab Allocator appends 64 bytes of metadata (or "Redzone" / Padding) to every 768-byte object in this specific cache configuration.
Legacy documentation claimed "Redzone moats" but implied the total footprint was derived purely from the struct size.
**Refined Truth:**
Physical Footprint = Struct Size (768) + Slab Overhead (64) = 832 bytes.
Efficiency: 92.3% Payload, 7.7% Overhead.
This overhead must be accounted for in memory capacity planning.
