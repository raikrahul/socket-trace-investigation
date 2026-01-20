#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/syscall.h>
int main() {
  printf("AF_INET=%d, SOCK_STREAM=%d, SYS_socket=%d\n", AF_INET, SOCK_STREAM,
         SYS_socket);
  return 0;
}
