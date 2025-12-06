#include "itec.h"

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


int main(int argc, char *argv[]) 
{   
    const char* port = "/dev/ttyUSB0";
    int serial_port = open(port, O_RDWR);

    if (serial_port < 0) {
        printf("Fehler beim Öffnen von %s: %s\n", port, strerror(errno));
        return 1;
    }

    int a = configure_com_port(port, serial_port);
    if (a != 0)
    {
        return 1;
    }

     sqlite3 *db;
    
    if (open_database(&db, "Messung1.db") != 0) 
    {
        return 1;
    }

    // Alte Tabelle löschen und neu erstellen
    execute_sql(db, "DROP TABLE IF EXISTS Messung1;");

    execute_sql(db,
        "CREATE TABLE IF NOT EXISTS Messung1 (Messung_Nr INTEGER PRIMARY KEY, Abstand_Real DOUBLE, Abstand_Sensor DOUBLE, Abweichung DOUBLE);");

    double gemessener_abstand[20];
    char eingelesen[100];

    printf("Bitte geben Sie 20 Messwerte ein (nach jedem Wert Enter drücken):\n\n");
    
    for (int i = 0; i < 20; i++) 
    {
        //Wert von der Kommandozeile einlesen
        printf("Wert %d: ", i + 1);
        
        if (fgets(eingelesen, sizeof(eingelesen), stdin) == NULL) {
            printf("Fehler beim Lesen!\n");
            return 1;
        }

        gemessener_abstand[i] = atof(eingelesen);  // Konvertiere String in double

        //Sensorwert einlesen
        char buffer[256];
        char temp[256];

        //mit Testwerten
        // double sensor_abstand = 1.5 + i;

        // double abweichung = gemessener_abstand[i] - sensor_abstand;

        // sprintf(temp, "INSERT INTO Messung1 (Abstand_Real, Abstand_Sensor, Abweichung) VALUES (%f,%f,%f);", gemessener_abstand[i], sensor_abstand, abweichung);
        // execute_sql(db, temp);

        //mit Sensorwerten 
        memset(buffer, 0, sizeof(buffer));

        int num_bytes = read(serial_port, buffer, sizeof(buffer));
        double sensor_abstand = atof(buffer);

        double abweichung = gemessener_abstand[i] - sensor_abstand;

        if (num_bytes < 0) {
            printf("Lesefehler: %s\n", strerror(errno));
            return 1;
        } else {
            sprintf(temp, "INSERT INTO Messung1 (Abstand_Real, Abstand_Sensor, Abweichung) VALUES (%f,%f,%f);", gemessener_abstand[i], sensor_abstand, abweichung);
            execute_sql(db, temp);
        }
    }

     //Tabelle in ein .csv File schreiben
    execute_sql_csv("Messung1.csv",db,
        "SELECT * FROM Messung1;");

    close(serial_port);
    sqlite3_close(db);
    
    return 0;

}
