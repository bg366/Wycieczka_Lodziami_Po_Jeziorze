#ifndef STERNIK_H
#define STERNIK_H

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_sternika(int lodz, int max_pomostu, int max_lodzi);

pid_t stworz_sternika(int lodz, int max_pomostu, int max_lodzi);

int zatrzymaj_sternika(pid_t pid_sternika);

#endif
