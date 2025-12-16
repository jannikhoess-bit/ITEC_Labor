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
#ifdef __APPLE__
#define OPEN_FLAGS O_NONBLOCK
#else
#define OPEN_FLAGS O_RDWR
#endif

/**
 * Configures the serial port with the desired settings.
 * 
 * Important settings are:
 * 
 * - Baudrate: 115200
 * - Initial read timeout: 0.5s
 * - no min bytes for read (non-blocking)
 * 
 * @param fd The file descriptor of the opened serial port.
 * @returns 0 on success, -1 on failure.
 */
int configure_serial(int fd) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    cfsetispeed(&tty, B115200);
    cfsetospeed(&tty, B115200);

    tty.c_cc[VTIME] = 5; // 0.5s Timeout
    tty.c_cc[VMIN] = 0; //has to be 0 for non-blocking read for mac os

    tcflush(fd, TCIFLUSH);

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr: Fehler beim Setzen der Einstellungen");
        return -1;
    }

    return 0;
}

/**
 * Handles a complete line read from the serial port.
 * @param line The line to handle (null-terminated string).
 * @returns the distance as float in cm, or -1.0f on error.
 */
float convert_to_sensor_val(const char *line) {
    char *endptr = NULL;
    errno = 0;
    float value = strtof(line, &endptr);

    if (!(errno == 0 && endptr != line)) {
        value = -1.0f;
        fprintf(stderr, "Warning: could not parse float from '%s' - Errno: %s\n", line, strerror(errno));
    }

    return value;
}


typedef struct _sim_data {
    size_t length;
    size_t index;
    float data[32];
} sim_data;

/**
 * Simulation of the sensor read
 * @return the number of bytes read
 */
ssize_t read_sim(int serial_fd,  char* chunk, size_t chunk_len){

    static sim_data s_data = {32, 0, 
        {10.25f, 12.80f, 15.60f, 18.45f, 21.10f, 23.95f, 26.70f, 29.55f,
        32.30f, 35.15f, 37.90f, 40.75f, 43.50f, 46.35f, 49.10f, 51.95f,
        54.70f, 57.55f, 60.30f, 63.15f, 65.90f, 68.75f, 71.50f, 74.35f,
        77.10f, 79.95f, 82.70f, 85.55f, 88.30f, 91.15f, 93.90f, 99.60f}};

    //sleep for 1 sec
    sleep(1);

    //neuen Werte aus liste holen
    float value = s_data.data[s_data.index];

    //update index
    s_data.index = (++s_data.index)%s_data.length; 


    if (chunk_len == 0) {
        return 0;
    }

    int written = snprintf(chunk, chunk_len, "%f\n", value);
    if (written < 0) {
        return -1;
    }

    if ((size_t)written >= chunk_len) {
        written = (int)(chunk_len - 1);
    }

    return written;
}

int connect_to_sensor(char * tty_path){
    //Oeffnen des seriellen Ports

    int serial_fd = open(tty_path, OPEN_FLAGS);

    if (serial_fd < 0) {
        fprintf(stderr, "Failed to open %s: %s\n", tty_path, strerror(errno));
        return -1;
    }

    //Konfiguration des seriellen Ports
    if (configure_serial(serial_fd) != 0) {
        fprintf(stderr, "Failed to configure %s: %s\n", tty_path, strerror(errno));
        close(serial_fd);
        return -1;
    }
    return serial_fd;
}

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