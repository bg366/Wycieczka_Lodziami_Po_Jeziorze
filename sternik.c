#include "sternik.h"
#include "utils/fifo.h"
#include "utils/interfejs.h"
#include "utils/pamiec_wspoldzielona.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

/*

stworz_sternika
usun_sternika

wypusc_pasazera

zacznij_rejs
zakoncz_rejs

przerwij_rejsy

*/

void logika_sternika(int lodz)
{
    printf(MAGENTA"[STERNIK %d] Rozpoczynam logikę sternika...\n"RESET, getpid());

    key_t key = ftok("/tmp", 'K');
    if (key == -1)
    {
        perror("ftok");
    }
    char fifo_str[20];
    snprintf(fifo_str, sizeof(fifo_str), "/tmp/lodz_%d", lodz);
    stworz_fifo(fifo_str);
    dane_wspolne_t* dw = dolacz_pamiec_wspoldzielona(key);

    int stop = 0;
    while(stop == 0)
    {
        if (dw->jest_koniec == 1) {
            printf(MAGENTA"[STERNIK %d] Jest koniec.\n"RESET, getpid());
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

        // tu będzie wpuszczać pasażerów

        // Sternik sprawdza ostatnią wiadomość
        int ilosc_vip = pobierz_liczbe_pasazerow(dw, lodz == 1 ? KOLEJKA_1_VIP : KOLEJKA_2_VIP);
        int ilosc_normalna = pobierz_liczbe_pasazerow(dw, lodz == 1 ? KOLEJKA_1_NORMALNA : KOLEJKA_2_NORMALNA);
        if (ilosc_vip == 0 && ilosc_normalna == 0) {
            sleep(3);
            continue;
        }
        printf(MAGENTA"[STERNIK %d] Przeszedłem dalej.\n"RESET, getpid());

        char osobisty_fifo_str[25];
        odczytaj_wiadomosc_z_fifo(fifo_str, osobisty_fifo_str, sizeof osobisty_fifo_str);
        printf(MAGENTA"[STERNIK %d] Odczytałem wiadomość.\n"RESET, getpid());

        // Sternik wysyła wiadomość do pasażera
        wyslij_wiadomosc_do_fifo(osobisty_fifo_str, "WPUSZCZONY");
        printf(MAGENTA"[STERNIK %d] Wpuściłem pasażera.\n"RESET, getpid());

        // Sternik czeka na odpowiedź
        char odpowiedz[20];
        odczytaj_wiadomosc_z_fifo(osobisty_fifo_str, odpowiedz, sizeof odpowiedz);
        printf(MAGENTA"[STERNIK %d] Odnotowane.\n"RESET, getpid());

        // tu będzie wyruszać w rejs

        // tu będzie wypuszczać pasażerów
    }
    sleep(2);
    usun_fifo(fifo_str);

    printf(MAGENTA"[STERNIK %d] Kończę.\n"RESET, getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

pid_t stworz_sternika(int lodz)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla sternika");
        return -1;
    }
    if (pid == 0)
    {
        // Proces potomny
        logika_sternika(lodz);
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
