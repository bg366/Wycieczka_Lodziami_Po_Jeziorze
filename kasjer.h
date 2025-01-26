#ifndef KASJER_H
#define KASJER_H

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "utils/czas.h"

void logika_kasjera(struct tm *godzina_zamkniecia);

pid_t stworz_kasjera(struct tm *godzina_zamkniecia);

int zatrzymaj_kasjera(pid_t pid_kasjera);

#endif
