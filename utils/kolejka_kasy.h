#ifndef KOLEJKA_KASY_H
#define KOLEJKA_KASY_H

#define KEY_FILEPATH "/tmp"
#define PROJ_ID  'K'
#include "../pasazer.h"

typedef struct {
    long mtype;
    pid_t pid;
    int wiek;
    int ma_dzieci;
    int wiek_dziecka;
    int preferowana_lodz;
    int powtarza_wycieczke;
} WiadomoscPasazera;

typedef struct {
    long mtype;
    int decyzja;
} OdpowiedzKasjera;

int stworz_kolejke(key_t klucz);

void usun_kolejke(key_t klucz);

int polacz_kolejke(key_t klucz);

int poinformuj_kasjera(int msgid, Pasazer *dane);

int poinformuj_pasazera(int msgid, OdpowiedzKasjera *odpowiedz);

int odbierz_wiadomosc_kasjera(int msgid, OdpowiedzKasjera *dane);

int odbierz_wiadomosc_pasazera(int msgid, WiadomoscPasazera *wiadomosc);

#endif
