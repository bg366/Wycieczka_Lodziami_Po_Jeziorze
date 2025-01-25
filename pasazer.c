#include "pasazer.h"
#include "utils/fifo.h"
#include "utils/interfejs.h"
#include "utils/kolejka_kasy.h"
#include "utils/pamiec_wspoldzielona.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/_types/_pid_t.h>
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

    printf(GREEN"[PASAZER %d] Rozpoczynam logikę pasażera...\n"RESET, getpid());

    key_t key = ftok(FTOK_PATH, 'K');
    if (key == -1)
    {
        perror("ftok");
    }
    int msgid = polacz_kolejke(key);

    while(1)
    {
        poinformuj_kasjera(msgid, dane);
        OdpowiedzKasjera odpowiedz;
        odbierz_wiadomosc_kasjera(msgid, &odpowiedz);

        if (!odpowiedz.decyzja) {
            printf(GREEN"[PASAZER %d] Nie wpuszczono mnie na rejs\n"RESET, getpid());
            break;
        }

        printf(GREEN"[PASAZER %d] Wpuszczono mnie na rejs\n"RESET, getpid());

        dane_wspolne_t *dw = dolacz_pamiec_wspoldzielona(key);
        identyfikator_kolejki_t kolejka;
        if (dane->preferowana_lodz == 1) {
            kolejka = dane->powtarza_wycieczke == 1 ? KOLEJKA_1_VIP : KOLEJKA_1_NORMALNA;
            dodaj_pasazera(dw, kolejka, getpid());
        } else {
            kolejka = dane->powtarza_wycieczke == 1 ? KOLEJKA_2_VIP : KOLEJKA_2_NORMALNA;
            dodaj_pasazera(dw, kolejka, getpid());
        }

        // Pasażer tworzy osobistą listę fifo
        char osobisty_fifo_str[25];
        snprintf(osobisty_fifo_str, sizeof(osobisty_fifo_str), "/tmp/pasazer_%d", getpid());
        stworz_fifo(osobisty_fifo_str);

        // Pasażer wysyła swój pid do sternika
        char fifo_str[20];
        snprintf(fifo_str, sizeof(fifo_str), "/tmp/lodz_%d", dane->preferowana_lodz);
        wyslij_wiadomosc_do_fifo(fifo_str, osobisty_fifo_str);
        printf(GREEN"[PASAZER %d] Wysyłam PID sternikowi.\n"RESET, getpid());

        // Pasażer czeka na wpuszczenie na statek
        char wiadomosc[20];
        odczytaj_wiadomosc_z_fifo(osobisty_fifo_str, wiadomosc, sizeof fifo_str);
        printf(GREEN"[PASAZER %d] Wchodzę na łódź.\n"RESET, getpid());
        pid_t pid = getpid();
        zdejmij_pasazera(dw, kolejka, &pid);

        wyslij_wiadomosc_do_fifo(osobisty_fifo_str, "WSZEDLEM");

        usun_fifo(osobisty_fifo_str);

        break;
    }

    sleep(1); // Symulacja krótkiej pracy
    printf(GREEN"[PASAZER %d] Kończę.\n"RESET, getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

void* logika_dziecka(void *arg) {
    Dziecko *dziecko = (Dziecko*)arg;

    printf(GREEN"[CHILD THREAD] Jestem dzieckiem pasażera %d, mam %d lat.\n"RESET,
           dziecko->id_rodzica, dziecko->wiek);

    for (int i = 0; i < 5; i++) {
        printf(GREEN"[CHILD THREAD] Dziecko pasażera %d czeka...\n"RESET, dziecko->id_rodzica);
        sleep(20);
    }

    printf(GREEN"[CHILD THREAD] Dziecko pasażera %d kończy swoje działanie.\n"RESET, dziecko->id_rodzica);

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

        pthread_t dziecko = NULL;

        if (dane.ma_dzieci == 1)
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
int zatrzymaj_pasazera(pid_t pid_pasazera)
{
    if (kill(pid_pasazera, SIGTERM) == -1)
    {
        perror("kill(pid_pasazera)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(pid_pasazera, &status, 0);
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
    printf(GREEN"[PARENT] Zakończono wątek dziecka (TID=%lu)\n"RESET, (unsigned long)dziecko_Tid);
    return 0;
}
