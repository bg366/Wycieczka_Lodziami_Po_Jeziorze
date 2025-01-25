#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include "kasjer.h"
#include "sternik.h"
#include "utils/interfejs.h"
#include "utils/generator_pasazerow.h"
#include "utils/kolejka_kasy.h"
#include "utils/pamiec_wspoldzielona.h"
#include "utils/sygnaly.h"

int main()
{
    key_t key = ftok(FTOK_PATH, 'K');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    int shmid = stworz_pamiec_wspoldzielona(key);
    dane_wspolne_t *dw = odbierz_dane_wspolne(shmid);
    inicjuj_dane_wspolne(dw);

    stworz_kolejke(key);

    stworz_kasjera();
    for (int i = 1; i <= 2; i++) {
        stworz_sternika(i);
    }

    pid_t generator = stworz_generator_pasazerow();
    sleep(5);
    zatrzymaj_generator_pasazerow(generator);
    sleep(5);


    dw->jest_koniec = 1;
    printf("Ustawiłem dw, %d\n", dw->jest_koniec);
    sleep(10);
    usun_kolejke(key);
    zakoncz_pamiec_wspoldzielona(dw, shmid);

    return 0;
}
