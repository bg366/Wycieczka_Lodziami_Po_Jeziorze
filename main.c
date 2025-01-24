#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include "kasjer.h"
#include "utils/interfejs.h"
#include "utils/generator_pasazerow.h"
#include "utils/kolejka_kasy.h"

int main()
{
    // int Tp, Tk;
    // int N1, N2;
    // int K;
    // int T1, T2;

    // setup_pamieci(&N1, &N2, &K);
    // setup_sternikow(&Tp, &Tk, &T1, &T2);

    key_t key = ftok("/tmp", 'K');
    if (key == -1) {
        perror("ftok");
        return 1;
    }
    stworz_kolejke(key);

    stworz_kasjera();
    pid_t generator = stworz_generator_pasazerow();
    sleep(5);
    zatrzymaj_generator_pasazerow(generator);

    usun_kolejke(key);

    // Tutaj można przekazać dane dalej do kolejnych etapów programu / logiki

    return 0;
}
