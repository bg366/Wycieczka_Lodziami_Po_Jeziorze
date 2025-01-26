#ifndef POLICJANT_H
#define POLICJANT_H

#include <sys/types.h>

/* Funkcja tworząca nowy proces policjanta (pojedynczy) */
pid_t stworz_policjanta();

/* Funkcja usuwająca proces pasażera */
int zatrzymaj_policjanta(pid_t policjant_pid);

/* Logika pasażera – co robi po utworzeniu?
   Wywoływana wewnątrz procesu policjanta. */
void logika_policjanta();

#endif
