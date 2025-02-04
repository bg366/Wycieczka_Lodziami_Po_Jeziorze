#include "pasazer.h"
#include "utils/fifo.h"
#include "utils/interfejs.h"
#include "utils/kolejka_kasy.h"
#include "utils/pamiec_wspoldzielona.h"
#include "utils/semafor.h"
#include "utils/kolejka_sternika.h"
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

static volatile sig_atomic_t flaga_zamkniecia_lodzi = 0;
static void sig_handler_lodz(int signo)
{
    (void)signo; // ignoruj param
    flaga_zamkniecia_lodzi = 1;
}

static volatile sig_atomic_t flaga_zejscia = 0;
static void sig_handler_term(int signo)
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

    int id_pomostu;
    int id_lodzi;
    identyfikator_kolejki_t kolejka;
    identyfikator_kolejki_t lodz;
    pid_t pid = getpid();

    // Pasażer tworzy osobistą listę fifo
    char osobisty_fifo_str[25];
    snprintf(osobisty_fifo_str, sizeof(osobisty_fifo_str), "/tmp/pasazer_%d", getpid());
    stworz_fifo(osobisty_fifo_str);


    int etap = 1;
    while(etap != 0)
    {
        if (etap == 1) {
            id_pomostu = dane->preferowana_lodz == 1 ? SEM_POMOST_1 : SEM_POMOST_2;
            id_lodzi = dane->preferowana_lodz == 1 ? SEM_LODZ_1 : SEM_LODZ_2;
            if (dane->preferowana_lodz == 1) {
                kolejka = dane->powtarza_wycieczke == 1 ? KOLEJKA_1_VIP : KOLEJKA_1_NORMALNA;
                lodz = LODZ_1;
            } else {
                kolejka = dane->powtarza_wycieczke == 1 ? KOLEJKA_2_VIP : KOLEJKA_2_NORMALNA;
                lodz = LODZ_2;
            }

            poinformuj_kasjera(msgid, dane);

            int wynik = 0;
            OdpowiedzKasjera odpowiedz;
            while (wynik == 0) {
                if (flaga_zamkniecia_lodzi == 1) {
                    etap = 0;
                    break;
                }
                wynik = odbierz_wiadomosc_kasjera(msgid, &odpowiedz);
            }

            if (etap == 0) {
                break;
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

            // Pasażer wysyła swój pid do sternika
            printf(GREEN"[PASAZER %d] Wysyłam PID sternikowi.\n"RESET, getpid());

            if (flaga_zamkniecia_lodzi == 1) {
                zdejmij_pasazera(dw, kolejka, &pid);
                break;
            }

            int wynik;
            wynik = s_poinformuj_sternika(msgid, dane);

            // Pasażer czeka na wpuszczenie na statek
            char wiadomosc[25];
            wynik = odczytaj_wiadomosc_z_fifo(osobisty_fifo_str, wiadomosc, sizeof wiadomosc);
            if (wynik != 0) {
                if (flaga_zamkniecia_lodzi == 1) {
                    zdejmij_pasazera(dw, kolejka, &pid);
                    break;
                }
                continue;
            }
            printf(GREEN"[PASAZER %d] Wchodzę na łódź.\n"RESET, getpid());

            // Wejście na łódź
            zdejmij_pasazera(dw, kolejka, &pid);
            opusc_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? -2 : -1);
            wyslij_wiadomosc_do_fifo(osobisty_fifo_str, "WSZEDLEM");

            // Wchodzenie przez pomost
            sleep(1);

            // Wejście na łódź
            opusc_semafor(semid, id_lodzi, dane->ma_dzieci == 1 ? -2 : -1);
            podnies_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? 2 : 1);
            dodaj_pasazera(dw, lodz, pid);
            printf(GREEN"[PASAZER %d] Jestem na łodzi %d.\n"RESET, getpid(), dane->preferowana_lodz);

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

            sleep(1); // symulacja przechodzenia przez pomost

            podnies_semafor(semid, id_pomostu, dane->ma_dzieci == 1 ? 2 : 1);
            printf(GREEN"[PASAZER %d] Zszedłem z pomostu.\n"RESET, getpid());

            etap = 4;
        }

        // logika decydowania czy chce powtorzyc podroz
        if (etap == 4) {
            if (losowa_liczba(1, 100) > 50 ? 1 : 0 && flaga_zamkniecia_lodzi != 1) {
                printf(GREEN"[PASAZER %d] Powtarzam wycieczkę\n"RESET, getpid());
                dane->powtarza_wycieczke = 1;
                dane->preferowana_lodz = losowa_liczba(1, 2);
                flaga_zejscia = 0;
                etap = 1;
                continue;
            } else {
                printf(GREEN"[PASAZER %d] Nie powtarzam wycieczki\n"RESET, getpid());
                break;
            }
        }
    }
    usun_fifo(osobisty_fifo_str);

    printf(GREEN"[PASAZER %d] Kończę.\n"RESET, getpid());
    exit(0); // Bezpieczne zakończenie procesu potomnego
}

void* logika_dziecka(void *arg) {
    Dziecko *dziecko = (Dziecko*)arg;

    for (int i = 0; i < 3; i++) {
        sleep(1);
    }

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

        signal(SIGTERM, sig_handler_term);
        signal(dane.preferowana_lodz == 1 ? SIGUSR1 : SIGUSR2, sig_handler_lodz);

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
        exit(0);
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
