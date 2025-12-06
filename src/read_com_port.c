// 22072025
// C-Programm zum Lesen eines µControllers über USB (Jetson Nano)
// Port: /dev/ttyUSB0
// Baudrate: 115200
// Endlosschleife zum Dauerlauf

#include "itec.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



int main() {
    const char* port = "/dev/ttyUSB0";
    int serial_port = open(port, O_RDWR);

    int a = configure_com_port(port, serial_port);
    if (a = 1)
    {
        return 1;
    }

    // Schleife zum Lesen
    while (1) {
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));

        int num_bytes = read(serial_port, buffer, sizeof(buffer));

        if (num_bytes < 0) {
            printf("Lesefehler: %s\n", strerror(errno));
        } else {
            printf("Gelesen: %s\n", buffer);
        }
    }

    close(serial_port);
    return 0;
}