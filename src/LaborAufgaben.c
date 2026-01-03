#include "itec.h"

#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

//Um die Simulation der Sensorwerte anzuschalten muss "SIM 0" gesetzt werden.
#define SIM 0


int main(int argc, char *argv[]) 
{   
    while (1) 
    {
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


        //Datenbank Verbindung öffnen
        sqlite3 *db;


        //Modus auswählen
        char modus[100];
        printf("\nWelchen Modus willst du nutzen[STOP, DA, TEST, AUTO, FILTER, FUN]: " );

        fgets(modus, sizeof(modus), stdin); 
        modus[strcspn(modus, "\n")] = '\0';

        if (strcmp(modus, "STOP") == 0) //Programm beenden
        {
            printf("Programm wird beendet.\n");
            break; 
        }
        else if (strcmp(modus, "DA") == 0) //Aufgabe 1
        {

            if (open_database(&db, "Messung1.db") != 0) 
            {
                return 1;
            }

            // Alte Tabelle löschen und neu erstellen
            execute_sql(db, "DROP TABLE IF EXISTS Messung1;");

            //Tabelle erstellen
            execute_sql(db,
                "CREATE TABLE IF NOT EXISTS Messung1 (Messung_Nr INTEGER PRIMARY KEY, Abstand_Real DOUBLE, Abstand_Sensor DOUBLE, Abweichung DOUBLE);");

            double gemessener_abstand[20];
            char eingelesen[100];

            printf("Bitte geben Sie 20 Messwerte in cm ein (nach jedem Wert Enter drücken):\n\n");


            int m = 0;
            while (m < 20) 
            {
                //Wert von der Kommandozeile einlesen
                printf("Wert %d: ", m + 1);

                if (fgets(eingelesen, sizeof(eingelesen), stdin) == NULL) {
                    printf("Fehler beim Lesen!\n");
                    return 1;
                }

                gemessener_abstand[m] = atof(eingelesen);  // Konvertiere String in double
               
                float sensorwert = -1.0f; 
                while (sensorwert < 0) 
                { 
                    sensorwert = read_sensor_value(serial_fd, chunk, line_buffer, &line_len); 
                }

                printf("Sensor Value: %.3f cm\n", sensorwert);
               
                //Abweichung berechnen
                double abweichung = gemessener_abstand[m] - sensorwert;

                char temp[256];

                //Werte in die Datenbank einfügen
                sprintf(temp, "INSERT INTO Messung1 (Abstand_Real, Abstand_Sensor, Abweichung) VALUES (%f,%f,%f);", gemessener_abstand[m], sensorwert, abweichung);
                execute_sql(db, temp);
                
                m++;
            }

            //Tabelle in ein .csv File schreiben
            execute_sql_csv("Messung1.csv",db,
                "SELECT * FROM Messung1;");

        } 
        else if (strcmp(modus, "TEST") == 0) //Aufgabe 2
        {
            
            //LookUp-Tabelle erstellen
            if (open_database(&db, "LookUpTabelle.db") != 0) 
            {
                return 1;
            }

            execute_sql(db, "DROP TABLE IF EXISTS LookUpTabelle;");

            execute_sql(db,
                "CREATE TABLE IF NOT EXISTS LookUpTabelle ("
                "Messung_Nr INTEGER PRIMARY KEY, "
                "Abstand_Real DOUBLE, "
                "Abstand_Sensor DOUBLE, "
                "Abweichung DOUBLE);");

            //LookUp importieren
            if (!import_lookup_from_csv(db, "Messung1.csv")) {
                printf("Fehler beim Importieren der LookUpTabelle!\n");
                return 1;
            }


            execute_sql(db, "DROP TABLE IF EXISTS Messung2;");

            execute_sql(db,
                "CREATE TABLE IF NOT EXISTS Messung2 (Messung_Nr INTEGER PRIMARY KEY, Abstand_Sensor DOUBLE, interpolierter_Wert DOUBLE, Abweichung DOUBLE);");

            
            printf("Drücke Enter um die Messung zu starten und q um die Messung abzubrechen...\n"); 

            int m = 1;
            while (1)
            {
                char enter[10];
                printf("Messung %d: ", m);
                fgets(enter, sizeof(enter), stdin);

                if (strcmp(enter, "q\n") == 0)
                    break;

                if (strcmp(enter, "\n") != 0) 
                {
                    printf("Ungültige Eingabe.\n");
                    continue;
                }

                // Sensorwert messen
                float sensorwert = -1.0f;
                while (sensorwert < 0)
                    sensorwert = read_sensor_value(serial_fd, chunk, line_buffer, &line_len);

                printf(" %.3f cm\n", sensorwert);

                // Verarbeitung ausgelagert
                interpolate_and_store_measurement(db, sensorwert);

                m++;
            }

            

            //Tabelle in ein .csv File schreiben
            execute_sql_csv("Messung2.csv",db,
                "SELECT * FROM Messung2;");
    
        } 
        else if (strcmp(modus, "AUTO") == 0) //Aufgabe 3
        {
            //Aufgabe 3 programmieren
            
        } 
        else if (strcmp(modus, "FILTER") == 0) //Aufgabe 4
        {
            //Aufgabe 4 programmieren 
           
        } 
        else if (strcmp(modus, "FUN") == 0) //Aufgabe 5
        {
            //Aufgabe 5 programmieren
            
        } 
        else 
        {
            printf("Ungültiger Modus. Bitte erneut versuchen.\n");
            continue;
        }
       
        #if SIM 
            close(serial_fd);
        #endif
        sqlite3_close(db);
        
    }
    
    return 0;

}








