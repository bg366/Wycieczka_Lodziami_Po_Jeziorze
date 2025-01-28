#include "sternik.h"
#include "utils/fifo.h"
#include "utils/interfejs.h"
#include "utils/kolejka_sternika.h"
#include "utils/pamiec_wspoldzielona.h"
#include "utils/semafor.h"
#include "utils/tablica_pid.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static volatile sig_atomic_t flaga_zejscia = 0;
static void sig_handler(int signo)
{
    (void)signo; // ignoruj param
    flaga_zejscia = 1;
}

void logika_sternika(int lodz, int max_pomostu, int max_lodzi, struct tm *godzina_zamkniecia, int czas_rejsu)
{
    printf(MAGENTA"[STERNIK %d] Rozpoczynam logikę sternika...\n"RESET, getpid());

    key_t key = ftok("/tmp", 'K');
    if (key == -1)
    {
        perror("ftok");
    }
    int msgid = s_polacz_kolejke(key);
    char fifo_str[20];
    char fifo_vip_str[20];
    snprintf(fifo_str, sizeof(fifo_str), "/tmp/lodz_%d", lodz);
    snprintf(fifo_vip_str, sizeof(fifo_str), "/tmp/lodz_v_%d", lodz);
    stworz_fifo(fifo_str);
    stworz_fifo(fifo_vip_str);
    dane_wspolne_t* dw = dolacz_pamiec_wspoldzielona(key);
    int semid = podlacz_semafor(key);

    int id_pomostu = lodz == 1 ? SEM_POMOST_1 : SEM_POMOST_2;
    int id_lodzi = lodz == 1 ? SEM_LODZ_1 : SEM_LODZ_2;

    tablica_pid *lista = utworz_tablice(4);

    int etap = 1;
    int stop = 0;
    int rejsy = 0;
    while(stop == 0)
    {
        if (flaga_zejscia == 1) {
            printf(MAGENTA"[STERNIK %d] Otrzymałem sygnał policji.\n"RESET, getpid());
            if (etap == 1) {
                etap = 3;
            }
        }
        if (czy_minela_godzina(godzina_zamkniecia)) {
            printf(MAGENTA"[STERNIK %d] Minął czas podróży.\n"RESET, getpid());
            if (flaga_zejscia != 1) {
                kill(0, lodz == 1 ? SIGUSR1 : SIGUSR2);
            }

            if (etap < 2) {
                etap = 3;
            }
        }
        if (dw->jest_koniec == 1) {
            stop = 1;
            break;
        }

        /*
            Setup sternika gdzie w pętli:
            1. Sprawcza czy nie minął czas rejsów i czy policja nie zatrzymała statku
            2. Zaczyna wpuszczać ludzi na łódź
            3. Wyrusza w rejs
            4. Wypuszcza ludzi z łodzi
        */

        // tu będzie check na czas i policje

        // tu sternik wpuszcza pasażerów na statek
        if (etap == 1) {
            int ilosc_vip = pobierz_liczbe_pasazerow(dw, lodz == 1 ? KOLEJKA_1_VIP : KOLEJKA_2_VIP);
            int ilosc_normalna = pobierz_liczbe_pasazerow(dw, lodz == 1 ? KOLEJKA_1_NORMALNA : KOLEJKA_2_NORMALNA);
            if (ilosc_vip == 0 && ilosc_normalna == 0) {
                printf(MAGENTA"[STERNIK %d] Brak ludzi.\n"RESET, getpid());
                sleep(1);
                continue;
            }

            int miejsce_na_lodzi = pobierz_wartosc_semafor(semid, id_lodzi);
            int miejsce_na_pomoscie = pobierz_wartosc_semafor(semid, id_pomostu);
            if (miejsce_na_pomoscie == 0) {
                printf(MAGENTA"[STERNIK %d] Brak miejsca.\n"RESET, getpid());
                sleep(1);
                continue;
            }
            if ((miejsce_na_lodzi - (max_pomostu - miejsce_na_pomoscie)) > 1) {

                if (flaga_zejscia == 1) {
                    printf(MAGENTA"[STERNIK %d] flaga zejscia.\n"RESET, getpid());
                    continue;
                }

                // Sternik sprawdza ostatnią wiadomość
                WiadomoscDoSternika wiadomosc;
                int kolejka = lodz == 1 ? 2 : 4;

                int wynik = s_odbierz_wiadomosc_pasazera(msgid, &wiadomosc, kolejka + 1);
                if (wynik == 0) {
                    wynik = s_odbierz_wiadomosc_pasazera(msgid, &wiadomosc, kolejka);
                    if (wynik == 0) {
                        sleep(1);
                        continue;
                    }
                }

                // Sternik wysyła wiadomość do pasażera
                char osobisty_fifo_str[25];
                snprintf(osobisty_fifo_str, sizeof(osobisty_fifo_str), "/tmp/pasazer_%d", wiadomosc.pid);
                wyslij_wiadomosc_do_fifo(osobisty_fifo_str, "WPUSZCZONY");

                // Sternik czeka na odpowiedź
                char odpowiedz[25];
                odczytaj_wiadomosc_z_fifo(osobisty_fifo_str, odpowiedz, sizeof odpowiedz);
                printf(MAGENTA"[STERNIK %d] Odnotowane.\n"RESET, getpid());

                dodaj_pid(lista, wiadomosc.pid);
            }
            else {
                if (miejsce_na_pomoscie != max_pomostu || flaga_zejscia == 1) {
                    sleep(1);
                    continue;
                }
                printf(MAGENTA"[STERNIK %d] Pełna łódź.\n"RESET, getpid());
                etap = 2;
            }
        }

        // Sternik wyrusza w rejs
        if (etap == 2) {
            sleep(czas_rejsu); // symulacja rejsu
            printf(MAGENTA"[STERNIK %d] Wróciliśmy.\n"RESET, getpid());
            rejsy++;
            etap = 3;
        }

        // Sterik wypuszcza pasażerów
        if (etap == 3) {
            if (flaga_zejscia == 0 && czy_minela_godzina(godzina_zamkniecia)) {
                kill(0, lodz == 1 ? SIGUSR1 : SIGUSR2);
            }
            while (lista->rozmiar > 0) {
                pid_t usuniety = usun_pid(lista);

                kill(usuniety, SIGTERM);
            }
            printf(MAGENTA"[STERNIK %d] Pasażerowie wypuszczeni.\n"RESET, getpid());
            if (czy_minela_godzina(godzina_zamkniecia) || flaga_zejscia == 1) {
                break;
            } else {
                while (pobierz_wartosc_semafor(semid, id_pomostu) != max_pomostu || pobierz_wartosc_semafor(semid, id_lodzi) != max_lodzi) { sleep(1); }
                printf(MAGENTA"[STERNIK %d] Zaczynam wpuszczać pasażerów.\n"RESET, getpid());
                etap = 1;
            }
        }
    }
    usun_fifo(fifo_str);
    usun_fifo(fifo_vip_str);

    usun_tablice(lista);

    printf(MAGENTA"[STERNIK %d] Kończę. Łódź nr %d odbyła %d rejsów\n"RESET, getpid(), lodz, rejsy);
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

pid_t stworz_sternika(int lodz, int max_pomostu, int max_lodzi, struct tm *godzina_zamkniecia, int czas_rejsu)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla sternika");
        return -1;
    }
    if (pid == 0)
    {
        signal(lodz == 1 ? SIGUSR1 : SIGUSR2, sig_handler);
        // Proces potomny
        logika_sternika(lodz, max_pomostu, max_lodzi, godzina_zamkniecia, czas_rejsu);
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
