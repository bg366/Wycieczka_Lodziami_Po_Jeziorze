#ifndef KASJER_H
#define KASJER_H

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_kasjera();

pid_t stworz_kasjera();

int zatrzymaj_kasjera(pid_t pid_kasjera);

#endif
