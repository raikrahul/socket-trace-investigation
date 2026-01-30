#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

/*
 * Operation Slab Anatomy
 * Goal: Discover why sock_inode_cache has 64 bytes overhead.
 * Method: Inspect kmem_cache flags by creating a REPLICA.
 */

static int __init slab_check_init(void) {
  struct kmem_cache *replica_cache;
  void *obj;
  int requested_size = 768; /* size of socket_alloc */

  /*
   * Replicate net/socket.c flags:
   * SLAB_HWCACHE_ALIGN (0x00002000)
   * SLAB_RECLAIM_ACCOUNT (0x00020000)
   * SLAB_ACCOUNT (0x04000000)
   * Removed SLAB_MEM_SPREAD (Deprecated/Removed in recent kernels)
   */
  unsigned long flags = SLAB_HWCACHE_ALIGN | SLAB_RECLAIM_ACCOUNT;
  /* Note: SLAB_ACCOUNT might also be implicit now, but safe to verify. */

  printk(KERN_INFO "AXIOM_SLAB: Creating Replica Cache (Size %d)...\n",
         requested_size);

  replica_cache =
      kmem_cache_create("axiom_replica", requested_size, 0, flags, NULL);

  if (!replica_cache) {
    printk(KERN_ERR "AXIOM_SLAB: Failed to create cache.\n");
    return -ENOMEM;
  }

  obj = kmem_cache_alloc(replica_cache, GFP_KERNEL);
  if (obj) {
    unsigned int actual_size = ksize(obj);
    printk(KERN_INFO "AXIOM_SLAB: Requested=%d, Actual=%u, Delta=%u\n",
           requested_size, actual_size, actual_size - requested_size);

    kmem_cache_free(replica_cache, obj);
  }

  kmem_cache_destroy(replica_cache);
  return 0;
}

static void __exit slab_check_exit(void) {
  printk(KERN_INFO "AXIOM_SLAB: Unloaded.\n");
}

module_init(slab_check_init);
module_exit(slab_check_exit);
