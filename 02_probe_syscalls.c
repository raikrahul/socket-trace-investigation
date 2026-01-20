#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/syscall.h>

int main() {
  printf("AF_INET=%d\n", AF_INET);
  printf("SOCK_STREAM=%d\n", SOCK_STREAM);
  printf("SYS_socket=%ld\n", SYS_socket);
  return 0;
}
