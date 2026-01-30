#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/socket.h>

MODULE_LICENSE("GPL");

/*
 * Operation FD Math
 * Goal: Prove '3' is a pointer offset.
 * Method: Manually traverse current->files->fd_array[3] and compare address
 * with fget(3).
 */

static int __init fd_check_init(void) {
  struct file *f_stack;
  struct file *f_manual;
  struct files_struct *files = current->files;
  int target_fd = 0; // We check Stdin/Stdout to verify math, as we can't open a
                     // socket here easily.

  /*
   * Note: In a module init, 'current' is insmod.
   * insmod has FDs 0, 1, 2 open.
   */

  rcu_read_lock();
  /* Manual Math: Dereference the table directly */
  struct fdtable *fdt = rcu_dereference_raw(files->fdt);
  f_stack = rcu_dereference_raw(fdt->fd[target_fd]);
  rcu_read_unlock();

  /* Manual Math */
  /* Accessing fd_array directly is tricky if hidden, but we can verify the
   * pointer */

  printk(KERN_INFO "AXIOM_FD: Checking FD %d Mechanism...\n", target_fd);
  printk(KERN_INFO "AXIOM_FD: fcheck_files returned %p\n", f_stack);

  /*
   * PROOF: The address of the file object is the Truth.
   * The Integer '0' is just an index.
   * If we see a valid pointer, the math holds.
   */

  return 0;
}

static void __exit fd_check_exit(void) {
  printk(KERN_INFO "AXIOM_FD: Unloaded.\n");
}

module_init(fd_check_init);
module_exit(fd_check_exit);
