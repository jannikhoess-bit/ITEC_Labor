#include "itec.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define READ_CHUNK 64

//Um die Simulation der Sensorwerte anzuschalten muss "SIM 0"  gesetzt werden.
#define SIM 1

//Praeprozessor Makro fuer die Oeffnungsflags des seriellen Ports
// #ifdef __APPLE__
// #define OPEN_FLAGS O_NONBLOCK
// #else
// #define OPEN_FLAGS O_RDWR
// #endif


int main(void) {
#if SIM
    char* tty_path = "/dev/ttyUSB0";
    int serial_fd = connect_to_sensor(tty_path);
#else
    int serial_fd = 0;    //wird in der Simulation nicht benoetigt.
#endif

    //Normalerweise wird stdout gepuffert, was bei der seriellen Ausgabe
    //zu Verzögerungen führen kann. Mit diesem Aufruf wird die Pufferung deaktiviert.
    setvbuf(stdout, NULL, _IONBF, 0);

    //Buffer zum Lesen und Verarbeiten von Daten pro Sensor
    char chunk[READ_CHUNK];
    char line_buffer[2*READ_CHUNK];
    size_t line_len = 0;

    while (1) {

        #if SIM
        ssize_t bytes_read = read(serial_fd, chunk, sizeof(chunk));
        #else
        ssize_t bytes_read = read_sim(serial_fd, chunk, sizeof(chunk));
        #endif

        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            break;
        }
        if (bytes_read == 0) {
            continue;
        }

        for (size_t i = 0; i < bytes_read; ++i) {
            char c = chunk[i];

            if (c == '\r') {
                continue;
            }

            if (c == '\n') {
                if (line_len > 0) {
                    line_buffer[line_len] = '\0';
                    float value = convert_to_sensor_val(line_buffer);
                    printf("Sensor Value: %.3f cm\n", value);

                    //TODO: Weiterverarbeiten des Sensorwerts
                    
                    line_len = 0;
                }
                continue;
            }

            if (line_len + 1 >= sizeof(line_buffer)) {
                fprintf(stderr, "Warning: incoming line too long, discarding.\n");
                line_len = 0;
                continue;
            }

            line_buffer[line_len++] = c;
        }
    }

    close(serial_fd);
    return 0;
}