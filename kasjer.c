#include "kasjer.h"
#include "utils/kolejka_kasy.h"
#include <stdio.h>
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
    int zarobek = 0;

    int i = 0;
    while(1)
    {
        WiadomoscPasazera wiadomosc;
        odbierz_wiadomosc_pasazera(msgid, &wiadomosc);

        OdpowiedzKasjera odpowiedz;
        odpowiedz.mtype = wiadomosc.pid;

        // logika decyzji
        if (wiadomosc.preferowana_lodz == 2 || wiadomosc.powtarza_wycieczke == 1)
        {
            odpowiedz.decyzja = 1;
        }
        else
        {
            if (wiadomosc.ma_dzieci || wiadomosc.wiek > 70)
            {
                odpowiedz.decyzja = 0;
            }
            else
            {
                odpowiedz.decyzja = 1;
            }
        }

        // logika zarobku
        int suma = 0;
        if (wiadomosc.wiek >= 3) suma+=2;
        if (wiadomosc.wiek_dziecka >= 3) suma+=2;
        if (wiadomosc.powtarza_wycieczke)
        {
            suma = suma / 2;
        }

        zarobek += suma;

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
int zatrzymaj_kasjera(pid_t pid_kasjera)
{
    if (kill(pid_kasjera, SIGTERM) == -1)
    {
        perror("kill(pid_kasjera)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(pid_kasjera, &status, 0);
    return 0;
}
