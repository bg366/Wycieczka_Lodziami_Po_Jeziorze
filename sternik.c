#include "sternik.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

/*

stworz_sternika
usun_sternika

zapros_pasazera
wypusc_pasazera

zacznij_rejs
zakoncz_rejs

przerwij_rejsy

*/

void logika_sternika(int lodz)
{
    printf("[STERNIK %d] Rozpoczynam logikę sternika...\n", getpid());

    key_t key = ftok("/tmp", 'K');
    if (key == -1)
    {
        perror("ftok");
    }

    int i = 0;
    while(1)
    {
        i++;
        sleep(1);
        if (i > 10)
        {
            break;
        }
    }

    sleep(1); // Symulacja krótkiej pracy
    printf("[STERNIK %d] Kończę.\n", getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

pid_t stworz_sternika(int lodz)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla sternika");
        return -1;
    }
    if (pid == 0)
    {
        // Proces potomny
        logika_sternika(lodz);
        // Nie powinno się tu dotrzeć, bo logika_sternika kończy proces
        _exit(0);
    }
    // Zwracamy PID dziecka
    return pid;
}

/* Funkcja do "zabicia" sternika np. gdybyśmy chcieli przedwcześnie przerwać jego działanie */
int zatrzymaj_sternika(pid_t pid_sternik)
{
    if (kill(pid_sternik, SIGTERM) == -1)
    {
        perror("kill(pid_sternik)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(pid_sternik, &status, 0);
    return 0;
}
