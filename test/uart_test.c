#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

int main() {
    int serial_port_num;
    char serial_port[15];
    
    printf("Select a serial port (3/4): ");
    scanf("%d", &serial_port_num);
    sprintf(serial_port,"/dev/ttyS%d",serial_port_num);
    int serial_fd;
    serial_fd = open(serial_port, O_RDWR | O_NOCTTY);
    if (serial_fd == -1) {
        perror("Failed to open serial port");
        return 1;
    }
    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(serial_fd, &tty) != 0) {
        perror("Error from tcgetattr");
        return 1;
    }

    cfsetospeed(&tty, B921600);
    cfsetispeed(&tty, B921600);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
        return 1;
    }

    char tx_buffer[] = "hello world!\n";
    ssize_t bytes_written = write(serial_fd, tx_buffer, sizeof(tx_buffer));
    if (bytes_written < 0) {
        perror("Error writing to serial port");
        close(serial_fd);
        return 1;
    }

    close(serial_fd);

    return 0;
}
