#ifndef SEMAFOR_H
#define SEMAFOR_H

#include <sys/types.h>
#include <sys/ipc.h>

int stworz_semafor(key_t klucz, int wartosc_poczatkowa);

int podlacz_semafor(key_t klucz);

int usun_semafor(int semid);

int pobierz_wartosc_semafor(int semid);

int opusc_semafor(int semid);

int podnies_semafor(int semid);

#endif
