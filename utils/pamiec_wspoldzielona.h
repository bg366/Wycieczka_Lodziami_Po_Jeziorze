#ifndef PAMIEC_WSPOLNA_H
#define PAMIEC_WSPOLNA_H

#include <sys/_types/_key_t.h>
#include <sys/_types/_pid_t.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

/* Stała określająca maksymalną liczbę pasażerów w jednej kolejce. */
#define MAX_PASAZEROW 100

/* Struktura kolejki FIFO w pamięci współdzielonej. */
typedef struct {
    pid_t dane[MAX_PASAZEROW];
    int poczatek;  /* indeks głowy kolejki (do zdejmowania) */
    int koniec;    /* indeks ogona kolejki (do dodawania)   */
    int liczba;    /* liczba elementów w kolejce           */
} kolejka_fifo_t;

/* Struktura pamięci współdzielonej */
typedef struct {
    kolejka_fifo_t kolejka_1_normalna;
    kolejka_fifo_t kolejka_1_vip;
    kolejka_fifo_t kolejka_2_normalna;
    kolejka_fifo_t kolejka_2_vip;
    kolejka_fifo_t lodz_1;
    kolejka_fifo_t lodz_2;

    pthread_mutex_t mutex_kolejka_1_normalna;
    pthread_mutex_t mutex_kolejka_1_vip;
    pthread_mutex_t mutex_kolejka_2_normalna;
    pthread_mutex_t mutex_kolejka_2_vip;
    pthread_mutex_t mutex_lodz_1;
    pthread_mutex_t mutex_lodz_2;
} dane_wspolne_t;

/*
 * Enum pozwalający wybrać daną kolejkę / łódź w funkcjach
 * operujących na tych strukturach.
 */
typedef enum {
    KOLEJKA_1_NORMALNA = 1,
    KOLEJKA_1_VIP      = 2,
    KOLEJKA_2_NORMALNA = 3,
    KOLEJKA_2_VIP      = 4,
    LODZ_1             = 5,
    LODZ_2             = 6
} identyfikator_kolejki_t;

int stworz_pamiec_wspoldzielona(key_t klucz);

dane_wspolne_t* dolacz_pamiec_wspoldzielona(key_t klucz);

int inicjuj_dane_wspolne(dane_wspolne_t *dw);

int dodaj_pasazera(dane_wspolne_t *dw, identyfikator_kolejki_t gdzie, pid_t pasazer);

int zdejmij_pasazera(dane_wspolne_t *dw, identyfikator_kolejki_t gdzie, pid_t *wynik);

int pobierz_liczbe_pasazerow(dane_wspolne_t *dw, identyfikator_kolejki_t gdzie);

int zakoncz_pamiec_wspoldzielona(dane_wspolne_t *dw, int shmid);

#endif
