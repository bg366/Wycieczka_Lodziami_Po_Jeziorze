#include "pasazer.h"
#include "utils/kolejka_kasy.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_pasazera(Pasazer *dane)
{
    /*
     * Logika pasażera:
     *  - Podłączenie się do pamięci wspólnej
     *  - Komunikacja z kasjerem (np. przez kolejkę komunikatów?)
     *  - Wejście na pomost (semafor)
     *  - Wejście na łódź, rejs, wyjście z łodzi, itp.
     */

    printf("[PASAZER %d] Rozpoczynam logikę pasażera...\n", getpid());

    key_t key = ftok("/tmp", 'K');
    if (key == -1)
    {
        perror("ftok");
    }
    int msgid = polacz_kolejke(key);

    while(1)
    {
        poinformuj_kasjera(msgid, dane);

        printf("PO POINFORMOWANIU\n");
        int i = 0;
        OdpowiedzKasjera odpowiedz;
        odbierz_wiadomosc_kasjera(msgid, &odpowiedz);

        printf("[PASAZER %d] Decyzja: %d\n", getpid(), odpowiedz.decyzja);

        break;
    }

    sleep(1); // Symulacja krótkiej pracy
    printf("[PASAZER %d] Kończę.\n", getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

void generuj_dane(Pasazer *dane)
{
    dane->wiek = 1;
    dane->ma_dzieci = 0;
    dane->preferowana_lodz = 1;
    dane->powtarza_wycieczke = 0;
}

pid_t stworz_pasazera()
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla pasażera");
        return -1;
    }
    if (pid == 0)
    {
        Pasazer dane;
        generuj_dane(&dane);
        // Proces potomny
        logika_pasazera(&dane);
        // Nie powinno się tu dotrzeć, bo logika_pasazera kończy proces
        _exit(0);
    }
    // Zwracamy PID dziecka
    return pid;
}

/* Funkcja do "zabicia" pasażera np. gdybyśmy chcieli przedwcześnie przerwać jego działanie */
int zatrzymaj_pasazera(pid_t kasjerPid)
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
