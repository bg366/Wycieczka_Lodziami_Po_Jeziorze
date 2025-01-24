#include "pasazer.h"
#include "utils/kolejka_kasy.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static int losowa_liczba(int a, int b) {
    return a + rand() % (b - a + 1);  // Ensures range [a, b]
}

void logika_pasazera(Pasazer *dane, pthread_t dziecko)
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

void* logika_dziecka(void *arg) {
    Dziecko *dziecko = (Dziecko*)arg;

    printf("[CHILD THREAD] Jestem dzieckiem pasażera %d, mam %d lat.\n",
           dziecko->id_rodzica, dziecko->wiek);

    for (int i = 0; i < 5; i++) {
        printf("[CHILD THREAD] Dziecko pasażera %d czeka...\n", dziecko->id_rodzica);
        sleep(20);
    }

    printf("[CHILD THREAD] Dziecko pasażera %d kończy swoje działanie.\n", dziecko->id_rodzica);

    free(dziecko);

    pthread_exit(NULL);
}

void generuj_dane(Pasazer *dane)
{
    dane->powtarza_wycieczke = 0;
    dane->preferowana_lodz = losowa_liczba(1, 2);
    dane->wiek = losowa_liczba(15, 80);
    dane->ma_dzieci = losowa_liczba(1, 50) > 40 && dane->wiek > 17 ? 1 : 0;
}

pthread_t stworz_dziecko(pid_t id_rodzica, int wiek_dziecka) {
    // Przygotuj dane do przekazania wątkowi
    Dziecko *dziecko = (Dziecko*)malloc(sizeof(Dziecko));
    if (!dziecko) {
        perror("malloc for ChildData");
        return 0;
    }
    dziecko->wiek = wiek_dziecka;
    dziecko->id_rodzica = id_rodzica;

    // Utwórz wątek
    pthread_t dziecko_Tid;
    int err = pthread_create(&dziecko_Tid, NULL, logika_dziecka, dziecko);
    if (err != 0) {
        fprintf(stderr, "pthread_create(child) error: %d\n", err);
        free(dziecko);
        return 0;
    }

    // Zwróć identyfikator wątku dziecka
    return dziecko_Tid;
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

        pthread_t dziecko;

        if (dane.ma_dzieci)
        {
            dane.wiek_dziecka = losowa_liczba(1, 14);
            dziecko = stworz_dziecko(getpid(), dane.wiek_dziecka);
        }
        else
        {
            dane.wiek_dziecka = 0;
        }

        // Proces potomny
        logika_pasazera(&dane, dziecko);
        // Nie powinno się tu dotrzeć, bo logika_pasazera kończy proces
        _exit(0);
    }
    // Zwracamy PID dziecka
    return pid;
}

/* Funkcja do "zabicia" pasażera np. gdybyśmy chcieli przedwcześnie przerwać jego działanie */
int zatrzymaj_pasazera(pid_t pasazer_Pid)
{
    if (kill(pasazer_Pid, SIGTERM) == -1)
    {
        perror("kill(pasazer_Pid)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(pasazer_Pid, &status, 0);
    return 0;
}

int zatrzymaj_dziecko(pthread_t dziecko_Tid) {
    // Przykład: czekamy aż dziecko się zakończy (join).
    // Jeżeli wątek już się zakończył, join i tak uprzątnie jego zasoby.
    int err = pthread_join(dziecko_Tid, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join(child) error: %d\n", err);
        return -1;
    }

    // Można dodać ewentualnie log:
    printf("[PARENT] Zakończono wątek dziecka (TID=%lu)\n", (unsigned long)dziecko_Tid);
    return 0;
}
