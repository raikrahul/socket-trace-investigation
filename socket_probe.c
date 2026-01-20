#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/sched.h>

#define TARGET_COMM "socket_test"

static struct kprobe kp_socket;

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // x86_64 Calling Convention: rdi, rsi, rdx
    int family = (int)regs->di;
    int type = (int)regs->si;
    int protocol = (int)regs->dx;

    pr_info("[PROBE] __sys_socket ENTRY: family=%d, type=%d, protocol=%d\n",
            family, type, protocol);
  }
  return 0;
}

static int __init probe_init(void) {
  kp_socket.pre_handler = handler_pre;
  kp_socket.symbol_name = "__sys_socket";

  if (register_kprobe(&kp_socket) < 0) {
    pr_err("register_kprobe failed\n");
    return -1;
  }

  pr_info("[PROBE] Single-Probe Driver (__sys_socket) Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kprobe(&kp_socket);
  pr_info("[PROBE] Single-Probe Driver Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
