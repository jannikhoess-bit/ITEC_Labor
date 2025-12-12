// 05122025
// C-Programm zum Lesen eines µControllers über USB (Jetson Nano)
// Port: /dev/ttyUSB0
// Baudrate: 115200
// 10x oder Endlosschleife zum Dauerlauf
// In der Wgile Schleife eigenes Program einfügen

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

    struct termios tty;

    // Baudrate setzen
    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    // 8N1 Konfiguration
    tty.c_cflag &= ~PARENB;        // Keine Parität
    tty.c_cflag &= ~CSTOPB;        // 1 Stopbit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;            // 8 Datenbits
    tty.c_cflag |= CREAD | CLOCAL; // Lesen aktivieren & lokaler Modus
  
    int k=0;   // Zaehler
    while (k<10) {       // Für Endlosschleife True einsetzten
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));

        int num_bytes = read(serial_port, buffer, sizeof(buffer));

        if (num_bytes < 0) {
            printf("Lesefehler: %s\n", strerror(errno));
        } else {
            printf("Abstand: %s \n", buffer);
            k=k+1;
        }
        // Hier eigenes Programm schreiben
    }

    close(serial_port);
    return 0;
}