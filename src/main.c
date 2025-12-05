#include <stdio.h>

#define MAX_LENGTH 100

int main(int argc, char *argv[]) {

    printf("Hello World with C version: %ld\n", __STDC_VERSION__);

    printf("Number of arguments: %i\n", argc);

    for(int i = 0; i < argc; i++) {
        printf("Argument %i: %s\n", i, argv[i]);    //
    }
    
    //Prüfen ob mindestens ein Argument übergeben wurde
    if (argc < 2) {
        printf("Please provide filename as an argument.\n");
        return 1;
    }

    //Datei öffnen die in argv[1] übergeben wurde
    FILE *file = fopen(argv[1], "r"); //"r" --> read mode

    //überprüfen ob die Datei geöffnet werden konnte
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LENGTH];
    int zeilenanzahl = 0;

    //Datei zeilenweise auslesen
    while (fgets(line, sizeof(line), file) != NULL){
        printf("%s\n", line);
        zeilenanzahl++;
    }

    printf("\nAnzahl der Zeilen: %i", zeilenanzahl);

    fclose(file);

    return 0;
}