#ifndef TABLICA_PID_H
#define TABLICA_PID_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct {
    pid_t *dane;
    size_t rozmiar;
    size_t pojemnosc;
} tablica_pid;

tablica_pid *utworz_tablice(size_t pojemnosc);

void dodaj_pid(tablica_pid *tablica, pid_t pid);

pid_t usun_pid(tablica_pid *tablica);

void usun_tablice(tablica_pid *tablica);

#endif
