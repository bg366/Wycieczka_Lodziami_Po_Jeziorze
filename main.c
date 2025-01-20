#include <stdio.h>
#include <stdlib.h>
#include "utils/interfejs.h"

/* gcc -Wall -o lodki main.c utils/interfejs.c -pthread */
int main() {
    int Tp, Tk;    // Czas początkowy i końcowy (np. godziny lub inny format)
    int N1, N2;    // Pojemność łodzi 1 i 2
    int K;         // Pojemność pomostu
    int T1, T2;    // Czas trwania rejsów (w sekundach)

    setup_pamieci(&N1, &N2, &K);
    setup_sternikow(&Tp, &Tk, &T1, &T2);

    // Tutaj można przekazać dane dalej do kolejnych etapów programu / logiki

    return 0;
}
