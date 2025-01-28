#include "kasjer.h"
#include "utils/kolejka_kasy.h"
#include "utils/interfejs.h"
#include "utils/czas.h"
#include "utils/pamiec_wspoldzielona.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static volatile sig_atomic_t flaga_zamkniecia_lodzi_1 = 0;
static void sig_handler_1(int signo)
{
    (void)signo; // ignoruj param
    flaga_zamkniecia_lodzi_1 = 1;
}

static volatile sig_atomic_t flaga_zamkniecia_lodzi_2 = 0;
static void sig_handler_2(int signo)
{
    (void)signo; // ignoruj param
    flaga_zamkniecia_lodzi_2 = 1;
}

void logika_kasjera(struct tm *godzina_zamkniecia)
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
    int ilosc = 0;
    int zaakceptowanych = 0;
    int odrzuconych = 0;
    while(stop == 0)
    {
        if (czy_minela_godzina(godzina_zamkniecia) || (flaga_zamkniecia_lodzi_1 == 1 && flaga_zamkniecia_lodzi_2 == 1)) {
            stop = 1;
            break;
        }

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
            odpowiedz.decyzja = flaga_zamkniecia_lodzi_2 == 0;
        }
        else
        {
            if ((wiadomosc.ma_dzieci || wiadomosc.wiek > 70) && wiadomosc.powtarza_wycieczke != 1)
            {
                odpowiedz.decyzja = 0;
            }
            else
            {
                odpowiedz.decyzja = flaga_zamkniecia_lodzi_1 == 0;
            }
        }

        // logika zarobku
        int suma = 0;
        if (wiadomosc.wiek >= 3) suma+=2;
        if (wiadomosc.wiek_dziecka >= 3) suma+=2;
        identyfikator_kolejki_t kolejka = wiadomosc.preferowana_lodz == 1 ? KOLEJKA_1_NORMALNA : KOLEJKA_2_NORMALNA;
        if (wiadomosc.powtarza_wycieczke)
        {
            kolejka = wiadomosc.preferowana_lodz == 1 ? KOLEJKA_1_VIP : KOLEJKA_2_VIP;
            suma = suma / 2;
        }

        if (pobierz_liczbe_pasazerow(dw, kolejka) >= MAX_PASAZEROW)
        {
            odpowiedz.decyzja = 0;
        }

        if (odpowiedz.decyzja == 1) {
            zarobek += suma;
        }

        if (odpowiedz.decyzja == 1) {
            zaakceptowanych++;
        } else {
            odrzuconych++;
        }

        ilosc++;

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
    printf(CYAN"[KASJER %d] Kończę. Ilość obsłużonych klientów: %d, zaakecptowanych: %d, odrzuconych: %d Zarobiliśmy: %d PLN\n"RESET, getpid(), ilosc, zaakceptowanych, odrzuconych, zarobek);
    _exit(0);
}

pid_t stworz_kasjera(struct tm *godzina_zamkniecia)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla kasjera");
        return -1;
    }
    if (pid == 0)
    {
        signal(SIGUSR1, sig_handler_1);
        signal(SIGUSR2, sig_handler_2);

        // Proces potomny
        logika_kasjera(godzina_zamkniecia);
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
