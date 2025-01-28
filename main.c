#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "kasjer.h"
#include "policjant.h"
#include "sternik.h"
#include "utils/interfejs.h"
#include "utils/generator_pasazerow.h"
#include "utils/kolejka_kasy.h"
#include "utils/pamiec_wspoldzielona.h"
#include "utils/semafor.h"
#include "utils/czas.h"

void handle_signal(int sig) {}

int main()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    int N1 = 5, N2 = 5, K = 4, T = 30, T1 = 2, T2 = 2;
    setup_pamieci(&N1, &N2, &K);
    setup_sternikow(&T, &T1, &T2);

    // Inicjalizacja IPC
    key_t key = ftok(FTOK_PATH, 'K');
    if (key == -1) {
        perror("ftok");
        return 1;
    }
    struct tm godzina = oblicz_godzine(T);
    int shmid = stworz_pamiec_wspoldzielona(key);
    dane_wspolne_t *dw = odbierz_dane_wspolne(shmid);
    inicjuj_dane_wspolne(dw);
    int semid = stworz_semafor(key, K, K, N1, N2);
    stworz_kolejke(key);

    // Procesy
    pid_t kasjer = stworz_kasjera(&godzina);
    pid_t sternik_1 = stworz_sternika(1, K, N1, &godzina, T1);
    pid_t sternik_2 = stworz_sternika(2, K, N2, &godzina, T2);
    pid_t generator = stworz_generator_pasazerow(&godzina);
    pid_t policjant = stworz_policjanta(&godzina);

    waitpid(policjant, NULL, 0);
    waitpid(generator, NULL, 0);
    waitpid(kasjer, NULL, 0);
    waitpid(sternik_1, NULL, 0);
    waitpid(sternik_2, NULL, 0);

    usun_kolejke(key);
    zakoncz_pamiec_wspoldzielona(dw, shmid);
    usun_semafor(semid);

    return 0;
}
