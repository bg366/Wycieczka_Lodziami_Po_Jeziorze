#ifndef KOLEJKA_FIFO_H
#define KOLEJKA_FIFO_H

#define FIFO_LODZ_1 = "/tmp/fifo/lodz/1"
#define FIFO_LODZ_2 = "/tmp/fifo/lodz/2"

#include <sys/types.h>

int stworz_fifo(const char *sciezka_fifo);

int wyslij_wiadomosc_do_fifo(const char *sciezka_fifo, char *str);

int odczytaj_wiadomosc_z_fifo(const char *sciezka_fifo, char *bufor, size_t rozmiar_bufora);

int zamknij_fifo(int deskryptor_fifo);

int usun_fifo(const char *sciezka_fifo);

#endif
