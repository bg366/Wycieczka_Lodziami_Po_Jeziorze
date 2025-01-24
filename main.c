#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include "kasjer.h"
#include "utils/interfejs.h"
#include "utils/generator_pasazerow.h"
#include "utils/kolejka_kasy.h"
#include "utils/pamiec_wspoldzielona.h"

int main()
{
    key_t key = ftok("/tmp", 'K');
    if (key == -1) {
        perror("ftok");
        return 1;
    }
    int shmid = stworz_pamiec_wspoldzielona(key);
    stworz_kolejke(key);

    stworz_kasjera();

    pid_t generator = stworz_generator_pasazerow();
    sleep(5);
    zatrzymaj_generator_pasazerow(generator);
    sleep(5);

    usun_kolejke(key);

    dane_wspolne_t *dw = dolacz_pamiec_wspoldzielona(shmid);
    zakoncz_pamiec_wspoldzielona(dw, shmid);

    return 0;
}
