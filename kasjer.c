#include "kasjer.h"
#include "utils/kolejka_kasy.h"
#include "utils/interfejs.h"
#include "utils/pamiec_wspoldzielona.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_kasjera()
{
    printf(CYAN"[KASJER %d] Rozpoczynam logikę kasjera...\n"RESET, getpid());

    key_t key = ftok(FTOK_PATH, 'K');
    if (key == -1)
    {
        perror("ftok");
    }
    int msgid = polacz_kolejke(key);
    dane_wspolne_t *dw = dolacz_pamiec_wspoldzielona(key);
    int zarobek = 0;

    int stop = 0;
    while(stop == 0)
    {
        WiadomoscPasazera wiadomosc;
        int wynik = odbierz_wiadomosc_pasazera(msgid, &wiadomosc);
        if (wynik != 1) {
            if (dw->jest_koniec == 1) {
                stop = 1;
                break;
            }
            continue;
        }

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

        wynik = poinformuj_pasazera(msgid, &odpowiedz);
        if (wynik != 1) {
            if (dw->jest_koniec == 1) {
                stop = 1;
                break;
            }
            continue;
        }
        printf(CYAN"[KASJER %d] Obsłużyłem pasażera %d.\n"RESET, getpid(), wiadomosc.pid);
    }
    printf(CYAN"[KASJER %d] Kończę.\n"RESET, getpid());
    _exit(0);
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
