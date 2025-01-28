#ifndef STERNIK_H
#define STERNIK_H

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "utils/czas.h"

void logika_sternika(int lodz, int max_pomostu, int max_lodzi, struct tm *godzina_zamkniecia, int czas_rejsu);

pid_t stworz_sternika(int lodz, int max_pomostu, int max_lodzi, struct tm *godzina_zamkniecia, int czas_rejsu);

int zatrzymaj_sternika(pid_t pid_sternika);

#endif
