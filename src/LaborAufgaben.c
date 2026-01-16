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
#include <stdbool.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>



//Um die Simulation der Sensorwerte anzuschalten muss "SIM 0" gesetzt werden.
//#define SIM 1

#define MAX_MEASUREMENTS 1000

double min_dt = 999999.0;
double max_dt = 0.0;
double sum_dt = 0.0;
long count_dt = 0;

void set_input_mode(void)
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void reset_input_mode(void)
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}



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
                // 1) Leere seriellen Eingabepuffer, damit keine alten Bytes stören
                tcflush(serial_fd, TCIFLUSH);

                // 2) Pufferzustand zurücksetzen
                line_len = 0;
                line_buffer[0] = '\0';

                // 3) Kleine Wartezeit, damit der Sensor frische Daten erzeugen kann
                usleep(200000); // 200 ms

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
            bool debug = true;

            //LookUp-Tabelle erstellen
            if (open_database(&db, "LookUpTabelle.db") != 0) 
            {
                return 1;
            }

            if (!create_lookup_table(db, "Messung1.csv")) 
            return 1;

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
                
                // 1) Leere seriellen Eingabepuffer, damit keine alten Bytes stören
                tcflush(serial_fd, TCIFLUSH);

                // 2) Pufferzustand zurücksetzen
                line_len = 0;
                line_buffer[0] = '\0';

                // 3) Kleine Wartezeit, damit der Sensor frische Daten erzeugen kann
                usleep(200000); // 200 ms
                while (sensorwert < 0)
                    sensorwert = read_sensor_value(serial_fd, chunk, line_buffer, &line_len);

                printf(" %.3f cm\n", sensorwert);

                // Verarbeitung ausgelagert
                double interpolated_value = Interpolate_measurement(db, sensorwert, debug);
                if (isnan(interpolated_value)) {
                    printf("Interpolation fehlgeschlagen.\n");
                    continue;
                }

                // Abweichung
                double abweichung = interpolated_value - sensorwert;

                // In DB einfügen
                char temp[256];
                sprintf(temp,
                    "INSERT INTO Messung2 (Abstand_Sensor, interpolierter_Wert, Abweichung) "
                    "VALUES (%f,%f,%f);",
                    sensorwert, interpolated_value, abweichung);

                execute_sql(db, temp);

                m++;
            }

            //Tabelle in ein .csv File schreiben
            execute_sql_csv("Messung2.csv",db,
                "SELECT * FROM Messung2;");
    
        } 


        else if (strcmp(modus, "AUTO") == 0) // Aufgabe 3
        {
            bool debug = false;
            min_dt = 999999.0;
            max_dt = 0.0;
            sum_dt = 0.0;
            count_dt = 0;

            struct timeval last_time, current_time;
            gettimeofday(&last_time, NULL);            

            // LookUp-Tabelle erstellen
            if (open_database(&db, "LookUpTabelle.db") != 0) 
                return 1;

            if (!create_lookup_table(db, "Messung1.csv"))
                return 1;

            execute_sql(db, "DROP TABLE IF EXISTS Messung3;");

            execute_sql(db,
                "CREATE TABLE IF NOT EXISTS Messung3 ("
                "Messung_Nr INTEGER PRIMARY KEY, "
                "Zeitstempel TEXT, "
                "Abstand_Sensor DOUBLE, "
                "interpolierter_Wert DOUBLE, "
                "Abweichung DOUBLE, "
                "ProfilHoehe DOUBLE);"); //Profielhöhe für Entwicklung mechatronischer Systeme

            printf("Automatische Messung gestartet. Drücke q zum Abbrechen...\n");

            set_input_mode();
            fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
            
            int start = 0;
  

            while (1)
            {
                char c;
                if (read(STDIN_FILENO, &c, 1) > 0)
                {
                    if (c == 'q')
                    {
                        printf("\nBeende AUTO-Modus...\n");
                        break;
                    }
                }

                // Sensorwert messen
                float sensorwert = -1.0f;
                while (sensorwert < 0)
                    sensorwert = read_sensor_value(serial_fd, chunk, line_buffer, &line_len);

                printf(" %.3f cm\n", sensorwert);

                gettimeofday(&current_time, NULL);

                double dt =
                    (current_time.tv_sec - last_time.tv_sec) +
                    (current_time.tv_usec - last_time.tv_usec) / 1e6;

                last_time = current_time;

                if (dt < min_dt) min_dt = dt;
                if (dt > max_dt) max_dt = dt;

                sum_dt += dt;
                count_dt++;


                // Interpolation
                double interpolated_value = Interpolate_measurement(db, sensorwert, debug);
                if (isnan(interpolated_value)) {
                    printf("Interpolation fehlgeschlagen.\n");
                    continue;
                }

                // Abweichung
                double abweichung = interpolated_value - sensorwert;

                //Profilhöhe für Entwicklung mechatronischer Systeme
                double Hoehe = 14;   //abstand Sensor zum Boden des profils
                if (interpolated_value > Hoehe) 
                {
                    interpolated_value = Hoehe;
                } //Vermeidung negativer Profilhöhe
                double profilhoehe = Hoehe - interpolated_value;

                // In DB einfügen
                char temp[256];
                sprintf(temp,
                    "INSERT INTO Messung3 (Zeitstempel, Abstand_Sensor, interpolierter_Wert, Abweichung, ProfilHoehe) "
                    "VALUES (DATETIME('now'),%f,%f,%f,%f);",
                    sensorwert, interpolated_value, abweichung, profilhoehe);

                execute_sql(db, temp);

                // Tabelle in CSV schreiben
                execute_sql_csv("Messung3.csv", db,
                    "SELECT * FROM Messung3;");

                // Histogramm-Array vorbereiten
                int histogram[MAX_MEASUREMENTS];
                int length = 0;

                FILE *fp = fopen("Messung3.csv", "r");
                if (!fp) {
                    printf("CSV konnte nicht geöffnet werden!\n");
                    return 1;
                }

                char line[256];

                // Header überspringen
                fgets(line, sizeof(line), fp);

                // CSV Zeilen einlesen
                while (fgets(line, sizeof(line), fp))
                {
                    int nr;
                    char timestamp[64];
                    float sensor, interpoliert, abw_csv, profilhoehe_csv;

                    if (sscanf(line, "%d,%[^,],%f,%f,%f,%f",
                            &nr, timestamp, &sensor, &interpoliert, &abw_csv, &profilhoehe_csv) == 6)
                    {
                        if (length >= MAX_MEASUREMENTS)
                            break;
                        
                        //int value = (int)round(interpoliert);   //Für Itec
                        if (profilhoehe_csv < 0) 
                            {profilhoehe_csv = 0;}
                        int value = (int)round(profilhoehe_csv);  //Für Entwicklung mechatronischer Systeme
                        if (value < 0) value = 0;

                        histogram[length++] = value;
                    }
                }

                fclose(fp);

                // Histogramm anzeigen
                struct winsize w;
                get_terminal_dim(&w);

                if (length <= w.ws_col)
                {
                    int end = length;
                    system("clear");
                    print_hist(length, histogram, start, end);
                }
                else
                {
                    int end = start + w.ws_col;
                    do 
                    {
                        system("clear");
                        print_hist(length, histogram, start, end);
                        start++;
                        end++;
                        usleep(100 * 500);
                    } while (end <= length);
                }
            }
            reset_input_mode();
            tcflush(STDIN_FILENO, TCIFLUSH);
            
            int flags = fcntl(STDIN_FILENO, F_GETFL); 
            flags &= ~O_NONBLOCK; 
            fcntl(STDIN_FILENO, F_SETFL, flags);

            printf("\n\n--- Abtastzeit-Statistik ---\n");
            printf("Minimale Abtastzeit: %.6f s\n", min_dt);
            printf("Maximale Abtastzeit: %.6f s\n", max_dt);

            if (count_dt > 0)
                printf("Durchschnittliche Abtastzeit: %.6f s\n", sum_dt / count_dt);
            else
                printf("Durchschnittliche Abtastzeit: keine Messungen\n");

            printf("-----------------------------\n");

            
            // ------------------------------------------------------------
            // Längenberechnung mit Uhrzeit-Eingabe (nur HH:MM:SS)
            // ------------------------------------------------------------
            
            char eingabe[100];
            double tA, tE;

            printf("Bitte Anfangszeit eingeben (Format: HH:MM:SS): ");
            fgets(eingabe, sizeof(eingabe), stdin);
            eingabe[strcspn(eingabe, "\n")] = 0;
            tA = parse_time_hms(eingabe);

            if (tA < 0) {
                printf("Ungueltige Uhrzeit!\n");
                return 1;
            }

            printf("Bitte Endzeit eingeben (Format: HH:MM:SS): ");
            fgets(eingabe, sizeof(eingabe), stdin);
            eingabe[strcspn(eingabe, "\n")] = 0;
            tE = parse_time_hms(eingabe);

            if (tE < 0) {
                printf("Ungueltige Uhrzeit!\n");
                return 1;
            }

            // Geschwindigkeit berechnen:
            float t1 = 183.0;   // Zeit für 80 cm
            float weg_cm = 80.0;
            float v = weg_cm / t1;   // cm/s

            // Zeitdifferenz
            double t = tE - tA;

            if (t < 0) {
                printf("Endzeit liegt vor der Anfangszeit!\n");
                return 1;
            }

            // Länge berechnen
            float laenge = v * t;

            printf("Länge des Bauteils: %.3f cm\n", laenge);
            

        }
       
        else if (strcmp(modus, "FILTER") == 0) // Aufgabe 4
        {
            // Öffne/erstelle DB für Filterergebnisse
            if (open_database(&db, "Messung4.db") != 0) {
                return 1;
            }

            // Alte Tabelle löschen und neue Tabelle anlegen
            execute_sql(db, "DROP TABLE IF EXISTS Messung4;");
            execute_sql(db,
                "CREATE TABLE IF NOT EXISTS Messung4 ("
                "Messung_Nr INTEGER PRIMARY KEY, "
                "Zeitstempel TEXT, "
                "Interpolierter_Wert DOUBLE, "
                "Interpolierter_Gefiltert DOUBLE);");

            // CSV einlesen
            const char *infile = "Messung3.csv";
            FILE *f = fopen(infile, "r");
            if (!f) {
                printf("Kann %s nicht öffnen. Stelle sicher, dass die Datei existiert.\n", infile);
                sqlite3_close(db);
                continue;
            }

            // Header überspringen
            char line[512];
            if (!fgets(line, sizeof(line), f)) {
                printf("Datei %s ist leer.\n", infile);
                fclose(f);
                sqlite3_close(db);
                continue;
            }

            // Dynamisches Array für interpolierte Werte und Timestamp
            size_t cap = 1024;
            size_t count = 0;
            double *interp_vals = malloc(cap * sizeof(double));
            char **timestamps = malloc(cap * sizeof(char*));
            int *nr_arr = malloc(cap * sizeof(int));
            if (!interp_vals || !timestamps || !nr_arr) {
                perror("malloc");
                fclose(f);
                free(interp_vals); free(timestamps); free(nr_arr);
                sqlite3_close(db);
                continue;
            }

            while (fgets(line, sizeof(line), f)) {
                // Erwartetes CSV-Format:
                // Messung_Nr,Zeitstempel,Abstand_Sensor,interpolierter_Wert,Abweichung,ProfilHoehe
                int nr = 0;
                char ts[128] = {0};
                double abstand = 0.0;
                double interp = 0.0, abw = 0.0, profil = 0.0;

                int scanned = sscanf(line, "%d,%127[^,],%lf,%lf,%lf,%lf",
                                    &nr, ts, &abstand, &interp, &abw, &profil);
                if (scanned < 4) {
                    // ungültige Zeile überspringen
                    continue;
                }

                if (count >= cap) {
                    size_t newcap = cap * 2;
                    double *tmpv = realloc(interp_vals, newcap * sizeof(double));
                    char **tmpts = realloc(timestamps, newcap * sizeof(char*));
                    int *tmpn = realloc(nr_arr, newcap * sizeof(int));
                    if (!tmpv || !tmpts || !tmpn) {
                        perror("realloc");
                        break;
                    }
                    interp_vals = tmpv;
                    timestamps = tmpts;
                    nr_arr = tmpn;
                    cap = newcap;
                }

                interp_vals[count] = interp;
                nr_arr[count] = nr;
                timestamps[count] = strdup(ts); // freed later
                count++;
            }
            fclose(f);

            if (count == 0) {
                printf("Keine gültigen Messdaten in %s gefunden.\n", infile);
                for (size_t i = 0; i < count; ++i) free(timestamps[i]);
                free(interp_vals); free(timestamps); free(nr_arr);
                sqlite3_close(db);
                continue;
            }

            // Gefilterte Werte berechnen (3-Punkt Mittelwert über interpolierte Werte)
            double *filtered = malloc(count * sizeof(double));
            if (!filtered) {
                perror("malloc");
                for (size_t i = 0; i < count; ++i) free(timestamps[i]);
                free(interp_vals); free(timestamps); free(nr_arr);
                sqlite3_close(db);
                continue;
            }

            for (size_t i = 0; i < count; ++i) {
                double y_prev = (i == 0) ? interp_vals[i] : interp_vals[i-1];
                double y_curr = interp_vals[i];
                double y_next = (i + 1 >= count) ? interp_vals[i] : interp_vals[i+1];
                filtered[i] = (y_prev + y_curr + y_next) / 3.0;
            }

            // Ergebnisse in DB einfügen (verwende interpolierten Wert und gefilterten Wert)
            char temp[512];
            for (size_t i = 0; i < count; ++i) {
                // Timestamp als Text einfügen; einfache SQL-Stringbildung, Timestamp sollte keine einfachen Hochkommas enthalten
                snprintf(temp, sizeof(temp),
                    "INSERT INTO Messung4 (Messung_Nr, Zeitstempel, Interpolierter_Wert, Interpolierter_Gefiltert) "
                    "VALUES (%d,'%s',%f,%f);",
                    nr_arr[i], timestamps[i], interp_vals[i], filtered[i]);
                execute_sql(db, temp);
            }

            // CSV aus DB erzeugen
            execute_sql_csv("Messung4.csv", db, "SELECT * FROM Messung4;");

            // Aufräumen
            for (size_t i = 0; i < count; ++i) free(timestamps[i]);
            free(interp_vals);
            free(timestamps);
            free(nr_arr);
            free(filtered);

            sqlite3_close(db);
            printf("FILTER-Modus abgeschlossen. Gefilterte Daten in Messung4.csv und Messung4.db\n");
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






