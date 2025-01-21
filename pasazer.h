#ifndef PASAZER_H
#define PASAZER_H

#include <sys/types.h>

/* Funkcja tworząca nowy proces pasażera (pojedynczy) */
pid_t stworz_pasazera();

/* Funkcja usuwająca proces pasażera */
int zatrzymaj_pasazera(pid_t passengerPid);

/* Logika pasażera – co robi po utworzeniu?
   Wywoływana wewnątrz procesu pasażera. */
void logika_pasazera();

#endif
