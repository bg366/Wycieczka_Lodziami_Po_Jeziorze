#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct {
    pid_t *dane;
    size_t rozmiar;
    size_t pojemnosc;
} tablica_pid;

tablica_pid *utworz_tablice(size_t pojemnosc) {
    tablica_pid *tablica = malloc(sizeof(tablica_pid));
    if (!tablica) exit(EXIT_FAILURE);

    tablica->dane = malloc(pojemnosc * sizeof(pid_t));
    if (!tablica->dane) {
        free(tablica);
        exit(EXIT_FAILURE);
    }

    tablica->rozmiar = 0;
    tablica->pojemnosc = pojemnosc;
    return tablica;
}

void dodaj_pid(tablica_pid *tablica, pid_t pid) {
    if (tablica->rozmiar >= tablica->pojemnosc) {
        tablica->pojemnosc *= 2;
        pid_t *nowe_dane = realloc(tablica->dane, tablica->pojemnosc * sizeof(pid_t));
        if (!nowe_dane) exit(EXIT_FAILURE);
        tablica->dane = nowe_dane;
    }

    tablica->dane[tablica->rozmiar++] = pid;
}

pid_t usun_pid(tablica_pid *tablica) {
    if (tablica->rozmiar == 0) return -1;
    return tablica->dane[--tablica->rozmiar];
}

void usun_tablice(tablica_pid *tablica) {
    free(tablica->dane);
    free(tablica);
}
