/*
01. INCLUDE_SECTION:
    - kprobes.h: For probing __sys_socket.
    - file.h: For file descriptor tables.
    - net.h: For socket structures.
*/
#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/sched.h>
#include <net/sock.h>

/*
02. CONSTANT_DEFINITION:
    - TARGET_COMM: "axiomatic_app" (Name of our user trigger)
*/
#define TARGET_COMM "axiomatic_app"

MODULE_LICENSE("GPL");

/*
03. GLOBAL_VARS:
    - rp_socket: Retprobe for __sys_socket
    - rp_sock_alloc: Retprobe for sock_alloc (to capture allocated address)
    - last_sock_addr: Store alloc address to verify against fd lookup
*/
static struct kretprobe rp_socket;
static struct kretprobe rp_sock_alloc;
static void *last_sock_addr = NULL;

/*
04. SOCK_ALLOC_RET:
    - Triggered when sock_alloc returns.
    - STORE the pointer in last_sock_addr.
*/
static int sock_alloc_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 0;

  last_sock_addr = (void *)regs->ax; /* CAPTURE RAX (Return Value) */
  pr_info("AXIOM: [sock_alloc] Return = %px\n", last_sock_addr);
  return 0;
}

/*
05. SYS_SOCKET_RET:
    - Triggered when __sys_socket returns.
    - CAPTURE FD (regs->ax).
    - DERIVE socket pointer from FD.
    - COMPARE with last_sock_addr.
*/
static int socket_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
  long fd = regs->ax;
  struct file *file;
  struct socket *sock_from_file;
  struct sock *sk;
  void *back_ptr;

  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 0;

  pr_info("AXIOM: [__sys_socket] Return FD = %ld\n", fd);

  /* 06. FD_LOOKUP: Check if FD is valid */
  if (fd < 0)
    return 0;

  rcu_read_lock();
  /* AXIOM: Accessing the fd array directly */
  {
    struct fdtable *fdt = files_fdtable(current->files);
    if (fd < fdt->max_fds)
      file = fdt->fd[fd];
    else
      file = NULL;
  }
  rcu_read_unlock();

  if (!file) {
    pr_err("AXIOM: ERROR: FD %ld not found in fdt\n", fd);
    return 0;
  }

  /* 07. DEREFERENCE_CHAIN: file -> private_data (socket) */
  sock_from_file = (struct socket *)file->private_data;
  pr_info("AXIOM: [fd->file->private_data] = %px\n", sock_from_file);

  /* 08. VERIFY_ALLOC: Does it match sock_alloc return? */
  if ((void *)sock_from_file == last_sock_addr) {
    pr_info("AXIOM: CHECK_1: sock_alloc result MATCHES fd lookup ✓\n");
  } else {
    pr_info("AXIOM: CHECK_1: MISMATCH ✗ (Alloc=%px vs FD=%px)\n",
            last_sock_addr, sock_from_file);
  }

  /* 09. DEREFERENCE_SOCK: socket -> sk */
  sk = sock_from_file->sk;
  pr_info("AXIOM: [socket->sk] = %px (Offset 24)\n", sk);

  if (sk) {
    /* 10. DEREFERENCE_BACK: sk -> sk_socket */
    back_ptr = sk->sk_socket;
    pr_info("AXIOM: [sk->sk_socket] = %px (Offset 288)\n", back_ptr);

    /* 11. VERIFY_CIRCLE: Does back pointer match socket? */
    if (back_ptr == (void *)sock_from_file) {
      pr_info("AXIOM: CHECK_2: Circular Link (sk->sk_socket == socket) "
              "VERIFIED ✓\n");
    } else {
      pr_info("AXIOM: CHECK_2: Circular Link BROKEN ✗\n");
    }
  }

  return 0;
}

/*
12. REGISTRATION:
    - Hook both functions.
*/
static int __init probe_init(void) {

  rp_socket.handler = socket_ret;
  rp_socket.kp.symbol_name = "__sys_socket";

  rp_sock_alloc.handler = sock_alloc_ret;
  rp_sock_alloc.kp.symbol_name = "sock_alloc";

  register_kretprobe(&rp_socket);
  register_kretprobe(&rp_sock_alloc);

  pr_info("AXIOM: Probe Loaded. Target: %s\n", TARGET_COMM);
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kretprobe(&rp_socket);
  unregister_kretprobe(&rp_sock_alloc);
  pr_info("AXIOM: Probe Unloaded.\n");
}

module_init(probe_init);
module_exit(probe_exit);
