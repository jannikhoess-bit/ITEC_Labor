#include <stdio.h>
#include <limits.h>

#include <sys/ioctl.h>
#include <unistd.h>

#include "itec.h"

int main(int argc, char *argv[])
{
    //long histogramm
    //int hist[] ={10,1,11,2,3,3,5,2,6,4,5,7,8,5,4,6,8,4,3,2,5,7,9,11,13,12,10,9,8,7,6,5,4,3,2,1,0,2,4,6,8,10,3,7,8,4,9,3,0,4,8,5,9,7,4,10,5,0,1,2,3,4,5,6,7,8,9,10,11,12,1,2,9,1,5,7,6,2,3,4,2,9,5,7,3,5,8,6,4,3,5,7,9,1,1,1,5,6,0,12,3,5,7,2,2,5,8,2,13,2,3,7,7,8,0,8,4,5,3,3,5,7,5,7,5,7,5,3,5,7,9,0,6,4,1,1,9,7,5,5,4,3,2,13,7,7,7,5,3,1,0,2,4,6,8,10,9,8,5,3,20,5,4,3,2,1,0,2,4,6,8,10,12,5,9,7,5,3,0,4,8,12,9,10,1,10,5,0,1,2,3,4,5,6,7,8,9,10,8,1,8,6,7,7,5,2,9,11};
    
    //histogramm
    int hist[] ={1,4,5,7,3,1,4,6,8,4,6,8,4,5,2,5,2,3,5,7,9,11,13,12,10,9,8,7,6,5,4,3,2,1,0,2,4,6,8,10,3,7,8,4,9,3,0,4,8,5,9,7,4,10,5,0,1,2,3};

    //short histogramm
    //int hist[] = {1,4,5,7,3,1,4,6,8,4,6,8,4,5,2,5};

    int size_of_hist = sizeof(hist);
    int size_of_hist_0 = sizeof(hist[0]);

    int length = size_of_hist / size_of_hist_0;

    struct array_static hist_s = {13, {1,4,5,7,3,1,4,6,8,4,6,8,4,5,2,5}};

    int max_hist_s = max_arr_struct(&hist_s);

    struct winsize w;
    get_terminal_dim(&w);

    int start = 0;
    int end = 0;

    //Histogramm visualization 

    if (length <= w.ws_col){
        end = length;
        system("clear");
        print_hist(length, hist, start, end);
    }
    else{
        end = w.ws_col;
        do {
            system("clear");
            print_hist(length, hist, start, end);
            start += 1;
            end += 1;
            usleep(100*1000); //sleep for 100 milliseconds
        } while (end <= length);
    }

    //Quicksort
    /*  printf("\n");
    quickSort(hist, 0, length - 1);
    print_hist(length, hist, 0, length); */

    //MergeSort
    printf("\n");
    mergeSort(hist, 0 , length - 1);
    print_hist(length, hist, 0, length);

    return 1;
}