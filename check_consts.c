#include <stdio.h>
#include <linux/net.h>
#include <linux/socket.h>

int main() {
    printf("AF_INET: %d\n", AF_INET);
    printf("NPROTO: %d\n", NPROTO);
    printf("sizeof(void*): %zu\n", sizeof(void*));
    return 0;
}
