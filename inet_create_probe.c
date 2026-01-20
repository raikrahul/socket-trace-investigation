#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>

#define TARGET_COMM "socket_test"

static struct kprobe kp_inet_create;

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // inet_create(struct net *net, struct socket *sock, ...)
    // 2nd arg is in rsi
    unsigned long sock_addr = regs->si;
    pr_info("[PROBE] inet_create ENTRY: socket_ptr=%lx\n", sock_addr);
  }
  return 0;
}

static int __init probe_init(void) {
  kp_inet_create.pre_handler = handler_pre;
  kp_inet_create.symbol_name = "inet_create";

  if (register_kprobe(&kp_inet_create) < 0) {
    pr_err("register_kprobe failed\n");
    return -1;
  }

  pr_info("[PROBE] inet_create Probe Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kprobe(&kp_inet_create);
  pr_info("[PROBE] inet_create Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
