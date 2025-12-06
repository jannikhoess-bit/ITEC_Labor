#ifndef ITEC_H  //If not defined
#define ITEC_H

#include <stdlib.h>
#include <sys/ioctl.h>

#include <sqlite3.h>

#define MAX_ARR_LENGHT 10000

struct array_static
{
    int lenght;
    int values[MAX_ARR_LENGHT];
};

//returns the maximum value of the array
int max_arr(int arr_lenght, int *arr);

//returns the index of the maximum value of the array
size_t max_arr_i(int arr_lenght, int *arr);

//returns the maximum value of the array structured
int max_arr_struct(struct array_static *arr);

//gets terminal dimensions
int get_terminal_dim (struct winsize *w);

//prints histogram to terminal
int print_hist(int arr_lenght, int *arr, int start, int end );

//swap 2 integers
void swap (int *a, int *b);

//partition for quick sort
int partition(int arr[], int low, int high);

//QuickSort
void quickSort(int arr[], int low, int high);

//MergeSort
void mergeSort(int arr[], int l, int r);

//merge for MergeSort
void merge(int arr[], int l, int m, int r);

//Datenbank öffnen
int open_database(sqlite3 **db, const char *db_name);

////In eine Datenbank schreiben und in die kommandozeile schreiben
int execute_sql(sqlite3 *db, const char *sql) ;

//In eine Datenbank schreiben und in eine .csv Datei schreiben
int execute_sql_csv(const char *filename, sqlite3 *db, const char *sql);

//Com Port vorbereiten
int configure_com_port(const char *port, int serial_port);



#endif //ITEC_H
