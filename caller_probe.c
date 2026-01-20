#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>

#define TARGET_COMM "socket_test"

static struct kprobe kp_caller;

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // AXIOM: Return Address is at [RSP] on function entry
    unsigned long *stack = (unsigned long *)regs->sp;
    unsigned long caller_addr = stack[0];

    pr_info("[TRICK] Function: %s\n", p->symbol_name);
    pr_info("[TRICK] My Address (IP): %lx\n", regs->ip);
    pr_info("[TRICK] Caller Address (from SP): %lx\n", caller_addr);
    pr_info("[TRICK] Symbolic Caller: %pS\n", (void *)caller_addr);
  }
  return 0;
}

static int __init probe_init(void) {
  kp_caller.pre_handler = handler_pre;
  kp_caller.symbol_name = "sock_alloc";

  if (register_kprobe(&kp_caller) < 0) {
    pr_err("register_kprobe failed\n");
    return -1;
  }

  pr_info("[TRICK] Caller-Capture Probe Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kprobe(&kp_caller);
  pr_info("[TRICK] Caller-Capture Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
