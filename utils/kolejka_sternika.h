#ifndef KOLEJKA_STERNIKA_H
#define KOLEJKA_STERNIKA_H

#define KEY_FILEPATH "/tmp"
#define PROJ_ID  'K'
#include "../pasazer.h"

typedef struct {
    long mtype;
    pid_t pid;
} WiadomoscDoSternika;

typedef struct {
    long mtype;
    int decyzja;
} OdpowiedzSternika;

int s_stworz_kolejke(key_t klucz);

void s_usun_kolejke(key_t klucz);

int s_polacz_kolejke(key_t klucz);

int s_poinformuj_sternika(int msgid, Pasazer *dane);

int s_poinformuj_pasazera(int msgid, OdpowiedzSternika *odpowiedz);

int s_odbierz_wiadomosc_sternika(int msgid, OdpowiedzSternika *dane);

int s_odbierz_wiadomosc_pasazera(int msgid, WiadomoscDoSternika *wiadomosc, int kolejka);

#endif
