#include "kolejka_sternika.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>

int s_stworz_kolejke(key_t klucz)
{
    int msgid = msgget(klucz, IPC_CREAT | 0600);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return msgid;
}

void s_usun_kolejke(key_t klucz)
{
    int msgid = msgget(klucz, 0);
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl(IPC_RMID)");
    }
}

int s_polacz_kolejke(key_t klucz)
{
    int msgid = msgget(klucz, 0600);
    if (msgid == -1) {
        perror("msgget (connect)");
        return -1;
    }
    return msgid;
}

int s_poinformuj_sternika(int msgid, Pasazer *dane)
{
    WiadomoscDoSternika wiadomosc;
    wiadomosc.mtype = (dane->preferowana_lodz == 1 ? 2 : 4) + dane->powtarza_wycieczke;
    wiadomosc.pid = getpid();

    // Wysłanie zgłoszenia do kasjera
    if (msgsnd(msgid, &wiadomosc, sizeof(WiadomoscDoSternika) - sizeof(long), 0) == -1)
    {
        if (errno == EIDRM) {
            printf("❌ Kolejka komunikatów została usunięta!\n");
            return -1;
        } else if (errno == EINVAL) {
            printf("❌ Niepoprawny `msgid` lub zbyt duża wiadomość.\n");
            return -1;
        } else if (errno == EAGAIN) {
            printf("❌ Kolejka pełna! Kasjer nie odebrał wcześniejszych wiadomości.\n");
            return 0;
        }
        exit(EXIT_FAILURE);
    }
    return 1;
}

int s_poinformuj_pasazera(int msgid, OdpowiedzSternika *odpowiedz)
{
    if (msgsnd(msgid, odpowiedz, sizeof(OdpowiedzSternika) - sizeof(long), 0) == -1)
    {
        if (errno == EIDRM) {
            printf("❌ Kolejka komunikatów została usunięta!\n");
            return -1;
        } else if (errno == EINVAL) {
            printf("❌ Niepoprawny `msgid` lub zbyt duża wiadomość.\n");
            return -1;
        } else if (errno == EAGAIN) {
            printf("❌ Kolejka pełna! Kasjer nie odebrał wcześniejszych wiadomości.\n");
            return 0;
        }
        exit(EXIT_FAILURE);
    }
    return 1;
}

int s_odbierz_wiadomosc_sternika(int msgid, OdpowiedzSternika *dane)
{
    if (msgrcv(msgid, dane, sizeof(OdpowiedzSternika) - sizeof(long), getpid(), IPC_NOWAIT) == -1)
    {
        if (errno == ENOMSG) {
            return 0;
        } else {
            perror("msgrcv");
            exit(EXIT_FAILURE);
            return -1;
        }
    }
    return 1;
}

int s_odbierz_wiadomosc_pasazera(int msgid, WiadomoscDoSternika *wiadomosc, int kolejka)
{
    if (msgrcv(msgid, wiadomosc, sizeof(WiadomoscDoSternika) - sizeof(long), kolejka, IPC_NOWAIT) == -1)
    {
        if (errno == ENOMSG) {
            return 0;
        } else {
            perror("msgrcv");
            exit(EXIT_FAILURE);
            return -1;
        }
    }
    return 1;
}
