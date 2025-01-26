#include "czas.h"

struct tm oblicz_godzine(int sekundy) {
    time_t teraz = time(NULL);
    teraz += sekundy;
    return *localtime(&teraz);
}

int czy_minela_godzina(struct tm *godzina) {
    time_t teraz = time(NULL);
    time_t czas_docelowy = mktime(godzina);
    return teraz >= czas_docelowy;
}
