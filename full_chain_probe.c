#include <linux/fdtable.h>
#include <linux/file.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/sched.h>
#include <net/sock.h>

#define TARGET_COMM "socket_test"

static struct kretprobe rp_socket;
static struct kretprobe rp_sock_alloc;

/* Capture __sys_socket return (the fd) */
static int socket_entry(struct kretprobe_instance *ri, struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 1;

  pr_info("=== SOCKET CHAIN START ===\n");
  pr_info("[1] __sys_socket ENTRY\n");
  pr_info("    PID: %d\n", current->pid);
  pr_info("    Args: family=%lu type=%lu protocol=%lu\n", regs->di, regs->si,
          regs->dx);
  pr_info("    current = %px\n", current);
  pr_info("    current->files = %px\n", current->files);
  return 0;
}

static int socket_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
  long fd = regs->ax;
  struct file *file;
  struct socket *sock;
  struct sock *sk;
  struct fdtable *fdt;

  pr_info("[6] __sys_socket RETURN\n");
  pr_info("    fd = %ld\n", fd);

  if (fd >= 0 && fd < 1024) {
    rcu_read_lock();
    fdt = files_fdtable(current->files);
    file = fdt->fd[fd];
    rcu_read_unlock();

    if (file) {
      pr_info("    fdt->fd[%ld] = %px\n", fd, file);
      pr_info("    file->f_op = %px\n", file->f_op);
      pr_info("    file->private_data = %px\n", file->private_data);

      sock = (struct socket *)file->private_data;
      if (sock) {
        pr_info("    socket = %px\n", sock);
        pr_info("    socket->state = %d\n", sock->state);
        pr_info("    socket->type = %d\n", sock->type);
        pr_info("    socket->sk = %px\n", sock->sk);
        pr_info("    socket->ops = %px\n", sock->ops);
        pr_info("    socket->file = %px\n", sock->file);

        sk = sock->sk;
        if (sk) {
          pr_info("    sock = %px\n", sk);
          pr_info("    sock->sk_family = %d\n", sk->sk_family);
          pr_info("    sock->sk_protocol = %d\n", sk->sk_protocol);
          pr_info("    sock->sk_state = %d\n", sk->sk_state);
          pr_info("    sock->sk_socket = %px\n", sk->sk_socket);
          pr_info("    sock->sk_prot = %px\n", sk->sk_prot);

          if (sk->sk_socket == sock) {
            pr_info("    BIDIRECTIONAL LINK VERIFIED\n");
          }
        }
      }
    }
  }

  pr_info("=== SOCKET CHAIN COMPLETE ===\n");
  return 0;
}

/* Capture sock_alloc return */
static int sock_alloc_entry(struct kretprobe_instance *ri,
                            struct pt_regs *regs) {
  if (strcmp(current->comm, TARGET_COMM) != 0)
    return 1;
  pr_info("[2] sock_alloc ENTRY\n");
  return 0;
}

static int sock_alloc_ret(struct kretprobe_instance *ri, struct pt_regs *regs) {
  struct socket *sock = (struct socket *)regs->ax;
  pr_info("[3] sock_alloc RETURN\n");
  pr_info("    socket = %px\n", sock);
  if (sock) {
    pr_info("    socket->state = %d\n", sock->state);
    pr_info("    socket->sk = %px (should be NULL)\n", sock->sk);
  }
  return 0;
}

static int __init probe_init(void) {
  int ret;

  rp_socket.handler = socket_ret;
  rp_socket.entry_handler = socket_entry;
  rp_socket.kp.symbol_name = "__sys_socket";
  ret = register_kretprobe(&rp_socket);
  if (ret < 0) {
    pr_err("Failed to register __sys_socket probe\n");
    return ret;
  }

  rp_sock_alloc.handler = sock_alloc_ret;
  rp_sock_alloc.entry_handler = sock_alloc_entry;
  rp_sock_alloc.kp.symbol_name = "sock_alloc";
  ret = register_kretprobe(&rp_sock_alloc);
  if (ret < 0) {
    pr_err("Failed to register sock_alloc probe\n");
    unregister_kretprobe(&rp_socket);
    return ret;
  }

  pr_info("[PROBE] Full Chain Probe LOADED\n");
  return 0;
}

static void __exit probe_exit(void) {
  unregister_kretprobe(&rp_socket);
  unregister_kretprobe(&rp_sock_alloc);
  pr_info("[PROBE] Full Chain Probe UNLOADED\n");
}

module_init(probe_init);
module_exit(probe_exit);
MODULE_LICENSE("GPL");
