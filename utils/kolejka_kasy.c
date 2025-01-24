#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include "kolejka_kasy.h"
#define MSG_KEY 5678

int stworz_kolejke(key_t klucz)
{
    int msgid = msgget(klucz, IPC_CREAT | 0600);
    if (msgid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return msgid;
}


void usun_kolejke(key_t klucz)
{
    int msgid = msgget(klucz, 0);
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl(IPC_RMID)");
    }
}

int polacz_kolejke(key_t klucz)
{
    int msgid = msgget(klucz, IPC_CREAT | 0600);
    if (msgid == -1) {
        perror("msgget (connect)");
        return -1;
    }
    return msgid;
}

void poinformuj_kasjera(int msgid, Pasazer *dane)
{
    WiadomoscPasazera wiadomosc;
    wiadomosc.mtype = 1;
    wiadomosc.pid = getpid();
    wiadomosc.ma_dzieci = dane->ma_dzieci;
    wiadomosc.powtarza_wycieczke = dane->powtarza_wycieczke;
    wiadomosc.preferowana_lodz = dane->preferowana_lodz;
    wiadomosc.wiek = dane->wiek;

    // Wysłanie zgłoszenia do kasjera
    if (msgsnd(msgid, &wiadomosc, sizeof(WiadomoscPasazera) - sizeof(long), 0) == -1)
    {
        perror("msgsnd");
        printf("🚨 Nie udało się wysłać wiadomości (PID: %d)\n", getpid());

        if (errno == EIDRM) {
            printf("❌ Kolejka komunikatów została usunięta!\n");
        } else if (errno == EINVAL) {
            printf("❌ Niepoprawny `msgid` lub zbyt duża wiadomość.\n");
        } else if (errno == EAGAIN) {
            printf("❌ Kolejka pełna! Kasjer nie odebrał wcześniejszych wiadomości.\n");
        }

        exit(EXIT_FAILURE);
    }
}

void poinformuj_pasazera(int msgid, OdpowiedzKasjera *odpowiedz)
{
    if (msgsnd(msgid, odpowiedz, sizeof(OdpowiedzKasjera) - sizeof(long), 0) == -1)
    {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }
}

void odbierz_wiadomosc_kasjera(int msgid, OdpowiedzKasjera *dane)
{
    if (msgrcv(msgid, dane, sizeof(OdpowiedzKasjera) - sizeof(long), getpid(), 0) == -1)
    {
        if (errno == ENOMSG) {
            printf("📭 Brak kasjera\n");
            exit(EXIT_SUCCESS);
        } else {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
    }
}

void odbierz_wiadomosc_pasazera(int msgid, WiadomoscPasazera *wiadomosc)
{
    printf("odbierz_wiadomosc_pasazera %d\n", msgid);
    if (msgrcv(msgid, wiadomosc, sizeof(WiadomoscPasazera) - sizeof(long), 1, 0) == -1)
    {
        if (errno == ENOMSG) {
            printf("📭 Brak wiadomości\n");
            return;
        } else {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
    }
}
