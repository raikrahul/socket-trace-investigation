#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/sched.h>

#define TARGET_COMM "socket_test"

/*
 * LATENCY & ERROR PROBE
 *
 * 1. STATEFUL PROBING: We need to remember "When did we start?"
 *    Solution: kretprobe allows us to allocate 'data' (a backpack)
 *    that travels with the function call.
 *
 * 2. TIMING:
 *    Axiom: ktime_get() returns current nanoseconds.
 *    Math: End - Start = Duration.
 *
 * 3. ERROR CHECKING:
 *    Axiom: In Kernel, integers > -4095 (0xfffff000) are Error Codes.
 *    (Because valid pointers/FDs are positive/lower).
 */

static struct kretprobe rp;

// The "Backpack" structure
struct my_data {
  ktime_t entry_timestamp;
};

// RUNS AT FUNCTION ENTRY (Start)
static int entry_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, "ping") == 0) {
    pr_info("[PROBE] SKIPPING PING | PID: %d | Returning 1 (No Kretprobe)\n",
            current->pid);
    return 1;
  }

  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 1; // Return 1 = Skip the Return Probe (Optimization)

  ((struct my_data *)ri->data)->entry_timestamp = ktime_get();

  // Log the Request (like before)
  pr_info("[PROBE] __sys_socket START | PID: %d | Args: %lld, %lld, %lld | TS: "
          "%lld\n",
          current->pid, regs->di, regs->si, regs->dx,
          ktime_to_ns(((struct my_data *)ri->data)->entry_timestamp));

  return 0;
}

// RUNS AT FUNCTION EXIT (End)
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs) {
  ktime_t now = ktime_get();
  s64 duration_ns = ktime_to_ns(
      ktime_sub(now, ((struct my_data *)ri->data)->entry_timestamp));
  long retval = regs->ax;

  // Error Axiom: IS_ERR_VALUE check (simplified)
  const char *status = (retval < 0 && retval > -4096) ? "FAIL" : "SUCCESS";

  pr_info("[PROBE] __sys_socket END   | PID: %d | FD: %ld | Status: %s | Time: "
          "%lld ns\n",
          current->pid, retval, status, duration_ns);

  return 0;
}

static int __init probe_init(void) {
  rp.handler = ret_handler;
  rp.entry_handler = entry_handler;
  rp.data_size = sizeof(struct my_data); /* Reserve space for backpack */
  rp.kp.symbol_name = "__sys_socket";

  if (register_kretprobe(&rp) < 0) {
    pr_err("register_kretprobe failed\n");
    return -1;
  }
  pr_info("[PROBE] Latency Probe Loaded.\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kretprobe(&rp);
  pr_info("[PROBE] Latency Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
