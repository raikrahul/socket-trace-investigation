#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/sched.h>

#define TARGET_COMM "socket_test"

/*
 * DUAL PROBE: ENTRY (kprobe) and EXIT (kretprobe)
 * PROOF OBJECTIVE:
 * 1. PRE:  Confirm Entry Args (2, 1, 0)
 * 2. POST: Confirm Success (Return Value > 0, i.e., File Descriptor)
 */

static struct kprobe kp_entry;
static struct kretprobe rp_exit;

// 1. ENTRY HANDLER (Arguments)
static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // x86_64: RDI=Arg1, RSI=Arg2, RDX=Arg3
    pr_info(
        "[PROBE] __sys_socket ENTRY: family=%lld, type=%lld, protocol=%lld\n",
        regs->di, regs->si, regs->dx);
  }
  return 0;
}

// 2. EXIT HANDLER (Return Value)
static int handler_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) == 0) {
    // x86_64: RAX=ReturnValue
    pr_info("[PROBE] __sys_socket EXIT:  fd=%ld\n", regs->ax);
  }
  return 0;
}

static int __init probe_init(void) {
  int ret;

  // Setup Entry Probe
  kp_entry.pre_handler = handler_pre;
  kp_entry.symbol_name = "__sys_socket";
  ret = register_kprobe(&kp_entry);
  if (ret < 0)
    return ret;

  // Setup Return Probe
  rp_exit.handler = handler_ret;
  rp_exit.kp.symbol_name = "__sys_socket";
  ret = register_kretprobe(&rp_exit);
  if (ret < 0) {
    unregister_kprobe(&kp_entry);
    return ret;
  }

  pr_info("[PROBE] Dual-Mode Driver Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kprobe(&kp_entry);
  unregister_kretprobe(&rp_exit);
  pr_info("[PROBE] Dual-Mode Driver Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
