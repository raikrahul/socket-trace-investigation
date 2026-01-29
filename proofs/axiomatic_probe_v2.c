/*
 * axiomatic_probe_v2.c
 * Purpose: Solve the "Harder Puzzle" - The inetsw Linked List
 * Axiom: inetsw is NOT a flat array. It is a Hash Table + Linked List.
 * Method: We receive the address of `inetsw` (found via kallsyms) and walk the
 * chain.
 */

#include <linux/init.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/net.h>
#include <net/protocol.h>

/* Parameter: Address of inetsw from /proc/kallsyms */
static unsigned long inetsw_addr = 0;
module_param(inetsw_addr, ulong, 0400);
MODULE_PARM_DESC(inetsw_addr, "Address of inetsw array");

/* Shadow struct definition if needed (though net/protocol.h should have it)
   We use the kernel's definition. */

static int __init probe_init(void) {
  struct list_head *inetsw_ptr;
  struct list_head *pos;
  struct inet_protosw *answer;
  int index = SOCK_STREAM; // We care about SOCK_STREAM (1)

  pr_info("=== AXIOM V2: INETSW TRAVERSAL START ===\n");

  if (inetsw_addr == 0) {
    pr_err("AXIOM: ERROR: inetsw_addr parameter is required.\n");
    return -EINVAL;
  }

  /* Cast the raw address to the list head array */
  inetsw_ptr = (struct list_head *)inetsw_addr;

  pr_info("AXIOM: inetsw address provided: 0x%lx\n", inetsw_addr);
  pr_info("AXIOM: Inspecting inetsw[%d] (SOCK_STREAM)\n", index);

  /* Point to the bucket for SOCK_STREAM */
  /* The array depends on SOCK_MAX. We trust the kernel headers. */
  struct list_head *head = &inetsw_ptr[index];

  /* Walk the list */
  list_for_each(pos, head) {
    /* container_of mapping: list -> struct inet_protosw */
    answer = list_entry(pos, struct inet_protosw, list);

    pr_info("AXIOM: Node Found at %p\n", answer);
    pr_info("AXIOM:   -> type     = %d (Expected 1)\n", answer->type);
    pr_info("AXIOM:   -> protocol = %d (IPPROTO_TCP=6, IPPROTO_SCTP=132)\n",
            answer->protocol);

    if (answer->prot) {
      pr_info("AXIOM:   -> prot->name = %s\n", answer->prot->name);
    } else {
      pr_info("AXIOM:   -> prot is NULL\n");
    }

    pr_info("AXIOM:   -> flags    = 0x%x\n", answer->flags);
  }

  pr_info("=== AXIOM V2: INETSW TRAVERSAL END ===\n");
  return 0;
}

static void __exit probe_exit(void) { pr_info("Axiom V2 probe unloaded.\n"); }

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
