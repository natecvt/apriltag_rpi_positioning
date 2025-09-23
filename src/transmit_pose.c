#include <transmit_pose.h>

int uart_init(int fd, char *interface, uint32_t baud) {
    fd = open(interface, O_RDWR | O_NOCTTY | O_NDELAY);

    if (fd < 0) {
        perror("Failed to open UART port");
        return 1;
    }

    struct termios sconf;

    if (tcgetattr(fd, &sconf)) {
        perror("Failed to set UART attributes pointer");
        return 2;
    }

    // baud rate, only use B----- pre-defined rates
    cfsetispeed(&sconf, baud);
    cfsetospeed(&sconf, baud);

    sconf.c_cflag &= ~PARENB;
    sconf.c_cflag &= ~CSTOPB;
    sconf.c_cflag &= ~CSIZE;

    sconf.c_cflag |= CS8; // use 8 data bits for easier concat later
    sconf.c_cflag |= CREAD | CLOCAL;

    if (tcsetattr(fd, TCSANOW, &sconf)) {
        perror("Failed to set UART attributes");
        return 3;
    }

    return 0;
}