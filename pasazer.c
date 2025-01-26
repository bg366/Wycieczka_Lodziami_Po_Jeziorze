#include "pasazer.h"
#include "utils/fifo.h"
#include "utils/interfejs.h"
#include "utils/kolejka_kasy.h"
#include "utils/pamiec_wspoldzielona.h"
#include "utils/semafor.h"
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
    srand(time(NULL) ^ (getpid()<<16));
    return a + rand() % (b - a + 1);  // Ensures range [a, b]
}

static volatile sig_atomic_t flaga_zejscia = 0;
static void sig_handler(int signo)
{
    (void)signo; // ignoruj param
    flaga_zejscia = 1;
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
    int semid = podlacz_semafor(key);
    dane_wspolne_t *dw = dolacz_pamiec_wspoldzielona(key);

    int id_pomostu = dane->preferowana_lodz == 1 ? SEM_POMOST_1 : SEM_POMOST_2;
    int id_lodzi = dane->preferowana_lodz == 1 ? SEM_LODZ_1 : SEM_LODZ_2;
    identyfikator_kolejki_t kolejka;
    identyfikator_kolejki_t lodz;
    pid_t pid = getpid();
    if (dane->preferowana_lodz == 1) {
        kolejka = dane->powtarza_wycieczke == 1 ? KOLEJKA_1_VIP : KOLEJKA_1_NORMALNA;
        lodz = LODZ_1;
    } else {
        kolejka = dane->powtarza_wycieczke == 1 ? KOLEJKA_2_VIP : KOLEJKA_2_NORMALNA;
        lodz = LODZ_2;
    }

    poinformuj_kasjera(msgid, dane);

    int etap = 1;
    while(1)
    {
        if (etap == 1) {
            int wynik = 0;
            OdpowiedzKasjera odpowiedz;
            while (wynik == 0) {
                sleep(2);
                wynik = odbierz_wiadomosc_kasjera(msgid, &odpowiedz);
            }

            if (odpowiedz.decyzja == 0) {
                printf(GREEN"[PASAZER %d] Nie wpuszczono mnie na rejs\n"RESET, getpid());
                break;
            }

            printf(GREEN"[PASAZER %d] Wpuszczono mnie na rejs\n"RESET, getpid());
            etap = 2;
        }

        if (etap == 2) {
            dodaj_pasazera(dw, kolejka, pid);

            // Pasażer tworzy osobistą listę fifo
            char osobisty_fifo_str[25];
            snprintf(osobisty_fifo_str, sizeof(osobisty_fifo_str), "/tmp/pasazer_%d", getpid());
            stworz_fifo(osobisty_fifo_str);

            // Pasażer wysyła swój pid do sternika
            char fifo_str[20];
            snprintf(fifo_str, sizeof(fifo_str), "/tmp/lodz_%d", dane->preferowana_lodz);
            printf(GREEN"[PASAZER %d] Wysyłam PID sternikowi. %d %d\n"RESET, getpid(), dane->ma_dzieci, dane->preferowana_lodz);
            wyslij_wiadomosc_do_fifo(fifo_str, osobisty_fifo_str);

            // Pasażer czeka na wpuszczenie na statek
            char wiadomosc[20];
            printf(GREEN"[PASAZER %d] Wchodzę na łódź.\n"RESET, getpid());
            odczytaj_wiadomosc_z_fifo(osobisty_fifo_str, wiadomosc, sizeof fifo_str);

            // Wejście na łódź
            opusc_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? -2 : -1);
            zdejmij_pasazera(dw, kolejka, &pid);
            // printf(GREEN"[PASAZER %d] Wszedłem na pomost%d.\n"RESET, getpid(), dane->preferowana_lodz);
            wyslij_wiadomosc_do_fifo(osobisty_fifo_str, "WSZEDLEM");

            // Wchodzenie przez pomost
            // printf(GREEN"[PASAZER %d] Czekam na semafor%d.\n"RESET, getpid(), dane->preferowana_lodz);

            // Wejście na łódź
            opusc_semafor(semid, id_lodzi, dane->ma_dzieci == 1 ? -2 : -1);
            podnies_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? 2 : 1);
            // printf(GREEN"[PASAZER %d] Czekam na pamiec%d.\n"RESET, getpid(), dane->preferowana_lodz);
            dodaj_pasazera(dw, lodz, pid);
            printf(GREEN"[PASAZER %d] Jestem na łodzi %d.\n"RESET, getpid(), dane->preferowana_lodz);

            usun_fifo(osobisty_fifo_str);

            etap = 3;
        }

        if (etap == 3) {
            if (flaga_zejscia != 1) {
                continue;
            }
            printf(GREEN"[PASAZER %d] Schodzę z łodzi.\n"RESET, getpid());
            // logika schodzenia ze statku
            opusc_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? -2 : -1);
            podnies_semafor(semid, id_lodzi, dane->ma_dzieci == 1 ? 2 : 1);
            zdejmij_pasazera(dw, lodz, &pid);

            sleep(5); // symulacja przechodzenia przez pomost

            podnies_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? 2 : 1);
            printf(GREEN"[PASAZER %d] Zszedłem z pomostu.\n"RESET, getpid());

            etap = 4;
        }

        // logika decydowania czy chce powtorzyc podroz
        if (etap == 4) {
            if (losowa_liczba(1, 100) > 50 ? 1 : 0) {
                dane->powtarza_wycieczke = 1;
                dane->preferowana_lodz = losowa_liczba(1, 2);
                flaga_zejscia = 0;
                etap = 1;
                continue;
            } else {
                break;
            }
        }

        break;
    }

    printf(GREEN"[PASAZER %d] Kończę.\n"RESET, getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

void* logika_dziecka(void *arg) {
    Dziecko *dziecko = (Dziecko*)arg;

    // printf(GREEN"[CHILD THREAD] Jestem dzieckiem pasażera %d, mam %d lat.\n"RESET,
           // dziecko->id_rodzica, dziecko->wiek);

    for (int i = 0; i < 5; i++) {
        // printf(GREEN"[CHILD THREAD] Dziecko pasażera %d czeka...\n"RESET, dziecko->id_rodzica);
        sleep(2);
    }

    // printf(GREEN"[CHILD THREAD] Dziecko pasażera %d kończy swoje działanie.\n"RESET, dziecko->id_rodzica);

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
        signal(SIGUSR1, sig_handler);
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
