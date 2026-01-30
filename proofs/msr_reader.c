#include <asm/msr.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

/*
 * MSR_LSTAR (0xC0000082) holds the Target RIP for the SYSCALL instruction.
 * This is the HARDWARE TRUTH of where the CPU jumps.
 */

static int __init msr_check_init(void) {
  unsigned long lstar;

  rdmsrl(MSR_LSTAR, lstar);

  printk(KERN_INFO "AXIOM_MSR: MSR_LSTAR (0xC0000082) = 0x%lx\n", lstar);
  return 0;
}

static void __exit msr_check_exit(void) {
  printk(KERN_INFO "AXIOM_MSR: Unloaded.\n");
}

module_init(msr_check_init);
module_exit(msr_check_exit);
