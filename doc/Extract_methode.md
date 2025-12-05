# Extract Methode

Ziel ist es Code in eine Methode auszulagern 
sodass dieser Code an mehreren Stellen aufgerufen werden kann

## Ausgangssituation

```c
int main(int argc, char* argv[]) {
    int x = 3;
    int y = 5;
    
    printf("y=%i, y=%i\n", x,y);

    //swap: tauschen von den Werten in x und y
    //TODO: Erstelle einne Methode swap
    //wir wollen nicht mehr kopieren sondern den richtigen wert ändern

    int temp = x;
    x=y;
    y=temp;

    printf("Swap: x= %i, y= %i\n", x,y);
}
```

## Zielsituation

Aufgabe: Methode 'swap' extrahieren, so dass man diese merhrmals verwenden kann.

```c
int swap(int *a, int *b) { //Pointer auf x und y übergeben
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char *argv[]) {
    int x = 3;
    int y = 5;
    
    printf("Start: x=%i, y=%i\n", x,y);

    swap(&x,&y); //& --> Adresse von x und y übergeben

    printf("Swap: x= %i, y= %i\n", x,y);
}

```