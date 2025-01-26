#ifndef SEMAFOR_H
#define SEMAFOR_H

#include <sys/types.h>
#include <sys/ipc.h>

typedef enum {
    SEM_POMOST_1 = 0,
    SEM_POMOST_2 = 1,
    SEM_LODZ_1 = 2,
    SEM_LODZ_2 = 3,
} identyfikator_semaforu_t;

int stworz_semafor(key_t klucz, int max_pomost_1, int max_pomost_2, int max_lodz_1, int max_lodz_2);

int podlacz_semafor(key_t klucz);

int usun_semafor(int semid);

int pobierz_wartosc_semafor(int semid, identyfikator_semaforu_t semafor);

int opusc_semafor(int semid, identyfikator_semaforu_t semafor, short ilosc);

int podnies_semafor(int semid, identyfikator_semaforu_t semafor, short ilosc);

#endif
