#include <stdio.h>

int swap(int *a, int *b) { //Pointer auf x und y übergeben
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char *argv[]) {  //argv[][] 
    int x = 3;
    int y = 5;
    
    printf("Start: x=%i, y=%i\n", x,y);

    swap(&x,&y); //& --> Adresse von x und y übergeben

    printf("Swap: x= %i, y= %i\n", x,y);

    int arr[2];

    arr[0] = 7;
    arr[1] = 8; //*(arr+1) = 8;

    int *arr_ptr = arr; //Pointer auf Anfang des Arrays

    int *arr_ptr0 = &arr[0]; //Adresse des ersten Elements
    int *arr_ptr1 = &arr[1]; //Adresse des zweiten Elements

    if ((arr_ptr0+1)==arr_ptr1){
        printf("is gleich\n");
    }

    //Ausgabe aller Argumente auf der Konsole
    for(int i = 0; i < argc; i++) {
        printf("Argument %i: %s\n", i, argv[i]);    

        //jedes Zeichen einzeln ausgeben
        //Ende des Strings is '\0'

        int j = 0;
        while (argv[i][j] != '\0') {
            printf("%c, ", argv[i][j]);
            j++;
        }
        printf("\n");
    }
    

    return 1;

}
