#include <sys/types.h>
#include <unistd.h>
#include "utils/interfejs.h"
#include "utils/generator_pasazerow.h"

int main() {
    int Tp, Tk;
    int N1, N2;
    int K;
    int T1, T2;

    // setup_pamieci(&N1, &N2, &K);
    // setup_sternikow(&Tp, &Tk, &T1, &T2);
    pid_t generator = stworz_generator_pasazerow();
    sleep(20);
    zatrzymaj_generator_pasazerow(generator);

    // Tutaj można przekazać dane dalej do kolejnych etapów programu / logiki

    return 0;
}
