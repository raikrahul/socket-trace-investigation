#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>

#define TARGET_COMM "socket_test"

static struct kprobe kp_security;

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // security_socket_create(family, type, protocol, kern)
    int family = (int)regs->di;
    pr_info("[PROBE] security_socket_create ENTRY: family=%d\n", family);
  }
  return 0;
}

static int __init probe_init(void) {
  kp_security.pre_handler = handler_pre;
  kp_security.symbol_name = "security_socket_create";

  if (register_kprobe(&kp_security) < 0) {
    pr_err("register_kprobe failed\n");
    return -1;
  }

  pr_info("[PROBE] Security Probe Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kprobe(&kp_security);
  pr_info("[PROBE] Security Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
