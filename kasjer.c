#include "kasjer.h"
#include "utils/kolejka_kasy.h"
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_kasjera()
{
    /*
     * Logika kasjera
     */

    printf("[KASJER %d] Rozpoczynam logikę kasjera...\n", getpid());

    key_t key = ftok("/tmp", 'K');
    if (key == -1)
    {
        perror("ftok");
    }
    int msgid = polacz_kolejke(key);

    int i = 0;
    while(1)
    {
        WiadomoscPasazera wiadomosc;
        odbierz_wiadomosc_pasazera(msgid, &wiadomosc);
        printf("‼️ DOSTALEM WIADOMOSC ‼️\n");

        OdpowiedzKasjera odpowiedz;
        odpowiedz.mtype = wiadomosc.pid;
        odpowiedz.decyzja = 1;

        poinformuj_pasazera(msgid, &odpowiedz);
        printf("[KASJER %d] Poinformowałem pasażera %d.\n", getpid(), wiadomosc.pid);
        i++;
        sleep(1);
        if (i > 10)
        {
            break;
        }
    }

    sleep(1); // Symulacja krótkiej pracy
    printf("[KASJER %d] Kończę.\n", getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

pid_t stworz_kasjera()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla kasjera");
        return -1;
    }
    if (pid == 0)
    {
        // Proces potomny
        logika_kasjera();
        // Nie powinno się tu dotrzeć, bo logika_kasjera kończy proces
        _exit(0);
    }
    // Zwracamy PID dziecka
    return pid;
}

/* Funkcja do "zabicia" kasjera np. gdybyśmy chcieli przedwcześnie przerwać jego działanie */
int zatrzymaj_kasjera(pid_t kasjerPid)
{
    if (kill(kasjerPid, SIGTERM) == -1)
    {
        perror("kill(kasjerPid)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(kasjerPid, &status, 0);
    return 0;
}
