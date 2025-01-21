#ifndef GENERATOR_PASAZEROW_H
#define GENERATOR_PASAZEROW_H

#include <sys/types.h>

/* Funkcja tworzy proces, który będzie w pętli generować pasażerów.
   Zwraca PID tego procesu (lub -1 w razie błędu). */
pid_t stworz_generator_pasazerow();

/* Funkcja zatrzymuje proces generatora pasażerów,
   np. przez wysłanie sygnału i/lub czekanie na zakończenie. */
int zatrzymaj_generator_pasazerow(pid_t generatorPid);

#endif
