/*
01. Include standard headers for I/O and System Calls.
    Axiom Source: /usr/include/
*/
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/*
02. Main Entry Point.
    Goal: Trigger __sys_socket in kernel.
*/
int main() {
  /*
  03. Call socket(AF_INET, SOCK_STREAM, 0).
      - AF_INET = 2 (IPv4 Internet protocols)
      - SOCK_STREAM = 1 (Sequenced, reliable, two-way connection-based byte
  streams)
      - Protocol = 0 (IPPROTO_IP - Dummy protocol for TCP)

      Derivation:
      - System Call: __NR_socket = 41
      - Register RDI = 2
      - Register RSI = 1
      - Register RDX = 0
  */
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  /*
  04. Check Result.
      - If fd >= 0: Success. Kernel allocated file, socket, and sock.
      - If fd < 0: Failure. Check errno.
  */
  if (fd < 0) {
    perror("socket failed");
    return 1;
  }

  /*
  05. Hold the process.
      - Allow us to inspect /proc/PID/fd if needed.
      - Allow probe to catch the return and log the fd.
  */
  printf("Socket created. FD = %d. Press ENTER to close...\n", fd);
  getchar();

  /*
  06. Close the file descriptor.
      - Triggers __close_fd -> sock_close -> sock_release.
  */
  close(fd);
  return 0;
}
