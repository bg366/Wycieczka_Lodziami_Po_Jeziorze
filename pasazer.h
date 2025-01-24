#ifndef PASAZER_H
#define PASAZER_H

#include <sys/_pthread/_pthread_t.h>
#include <sys/types.h>

typedef struct {
    int wiek;
    int ma_dzieci;
    int wiek_dziecka;
    int preferowana_lodz;
    int powtarza_wycieczke;
} Pasazer;

typedef struct {
    int wiek;
    pid_t id_rodzica;
} Dziecko;

/* Funkcja tworząca nowy proces pasażera (pojedynczy) */
pid_t stworz_pasazera();

/* Funkcja usuwająca proces pasażera */
int zatrzymaj_pasazera(pid_t pid_pasazera);

/* Logika pasażera – co robi po utworzeniu?
   Wywoływana wewnątrz procesu pasażera. */
void logika_pasazera(Pasazer *dane, pthread_t dziecko);

#endif
