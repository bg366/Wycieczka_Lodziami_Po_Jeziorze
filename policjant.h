#ifndef POLICJANT_H
#define POLICJANT_H

#include <time.h>
#include <sys/types.h>

/* Funkcja tworząca nowy proces policjanta (pojedynczy) */
pid_t stworz_policjanta(struct tm *godzina_zamkniecia);

/* Funkcja usuwająca proces pasażera */
int zatrzymaj_policjanta(pid_t policjant_pid);

/* Logika pasażera – co robi po utworzeniu?
   Wywoływana wewnątrz procesu policjanta. */
void logika_policjanta(struct tm *godzina_zamkniecia);

#endif
