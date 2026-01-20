#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

int main() { socket(AF_INET, SOCK_STREAM, 0); }
