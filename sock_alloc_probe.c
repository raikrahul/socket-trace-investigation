#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/socket.h>

#define TARGET_COMM "socket_test"

static struct kretprobe rp_sock_alloc;

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // x86_64 Return Value: rax
    unsigned long sock_addr = regs->ax;
    pr_info("[PROBE] sock_alloc EXIT: socket_ptr=%lx\n", sock_addr);
  }
  return 0;
}

static int __init probe_init(void) {
  rp_sock_alloc.handler = ret_handler;
  rp_sock_alloc.kp.symbol_name = "sock_alloc";

  if (register_kretprobe(&rp_sock_alloc) < 0) {
    pr_err("register_kretprobe failed\n");
    return -1;
  }

  pr_info("[PROBE] Return-Probe (sock_alloc) Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kretprobe(&rp_sock_alloc);
  pr_info("[PROBE] Return-Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
