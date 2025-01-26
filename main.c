#include <sys/ipc.h>
#include <sys/types.h>
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

int main()
{
    int N1 = 5, N2 = 2, K = 2, T = 10;
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
    stworz_policjanta();
    stworz_kasjera(&godzina);
    for (int i = 1; i <= 2; i++) {
        stworz_sternika(i, K, i == 1 ? N1 : N2, &godzina);
    }
    pid_t generator = stworz_generator_pasazerow(&godzina);

    // Clean-up
    sleep(30);
    dw->jest_koniec = 1;
    printf("Ustawiłem dw, %d\n", dw->jest_koniec);
    sleep(10);
    usun_kolejke(key);
    zakoncz_pamiec_wspoldzielona(dw, shmid);
    usun_semafor(semid);

    return 0;
}
