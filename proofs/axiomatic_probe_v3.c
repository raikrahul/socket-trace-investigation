/*
 * axiomatic_probe_v3.c
 * Purpose: Solve the "Slab Geometry Puzzle"
 * Axiom: sock_inode_cache size is 832 bytes.
 * Question: Why 832? (Legacy claimed 768).
 * Method: Print sizeof(struct inode) and sizeof(struct socket_alloc).
 */

#include <linux/fs.h> // struct inode
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/socket.h> // struct socket
#include <net/sock.h>     // struct socket_alloc

static int __init probe_init(void) {
  pr_info("=== AXIOM V3: SLAB GEOMETRY START ===\n");

  pr_info("AXIOM: sizeof(struct socket)       = %lu\n", sizeof(struct socket));
  pr_info("AXIOM: sizeof(struct inode)        = %lu\n", sizeof(struct inode));

  /* define from include/net/sock.h if needed, but it should be there */
  /* struct socket_alloc { struct socket socket; struct inode vfs_inode; } */

  pr_info("AXIOM: sizeof(struct socket_alloc) = %lu\n",
          sizeof(struct socket_alloc));

  /* Calculate padding */
  size_t theoretical = sizeof(struct socket) + sizeof(struct inode);
  pr_info("AXIOM: Raw Sum (socket + inode)    = %lu\n", theoretical);
  pr_info("AXIOM: Delta (alloc - sum)         = %lu\n",
          sizeof(struct socket_alloc) - theoretical);

  /* Alignment Check */
  pr_info("AXIOM: offsetof(struct socket_alloc, vfs_inode) = %lu\n",
          offsetof(struct socket_alloc, vfs_inode));

  pr_info("=== AXIOM V3: SLAB GEOMETRY END ===\n");
  return 0;
}

static void __exit probe_exit(void) { pr_info("Axiom V3 probe unloaded.\n"); }

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
