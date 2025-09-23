#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int uart_init(int fd, char *interface, uint32_t baud);