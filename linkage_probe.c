#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/sched.h>

#define TARGET_COMM "socket_test"

static struct kprobe kp_linkage;

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    pr_info("[FORENSIC] sock_alloc Hit. Proving Call Chain...\n");
    dump_stack();
  }
  return 0;
}

static int __init probe_init(void) {
  kp_linkage.pre_handler = handler_pre;
  kp_linkage.symbol_name = "sock_alloc";

  if (register_kprobe(&kp_linkage) < 0) {
    pr_err("register_kprobe failed\n");
    return -1;
  }

  pr_info("[FORENSIC] Linkage Probe Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kprobe(&kp_linkage);
  pr_info("[FORENSIC] Linkage Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
