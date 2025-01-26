#include "fifo.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int stworz_fifo(const char *sciezka_fifo) {
    if (mkfifo(sciezka_fifo, 0666) == -1) {
        perror("Błąd tworzenia FIFO");
        return -1;
    }
    return 0;
}

int wyslij_wiadomosc_do_fifo(const char *sciezka_fifo, char *str) {
    int deskryptor_fifo = open(sciezka_fifo, O_WRONLY);
    if (deskryptor_fifo == -1) {
        return -1;
    }

    if (write(deskryptor_fifo, str, strlen(str) + 1) == -1) {
        perror("Błąd zapisu do FIFO");
        close(deskryptor_fifo);
        return -1;
    }

    close(deskryptor_fifo);
    return 0;
}

int odczytaj_wiadomosc_z_fifo(const char *sciezka_fifo, char *bufor, size_t rozmiar_bufora) {
    int deskryptor_fifo = open(sciezka_fifo, O_RDONLY);
    if (deskryptor_fifo == -1) {
        return -1;
    }

    ssize_t bajty_odczytane = read(deskryptor_fifo, bufor, rozmiar_bufora);
    if (bajty_odczytane == -1) {
        perror("Błąd odczytu z FIFO");
        close(deskryptor_fifo);
        return -1;
    }

    bufor[bajty_odczytane] = '\0'; // Dodanie null-terminatora
    close(deskryptor_fifo);
    return 0;
}

int zamknij_fifo(int deskryptor_fifo) {
    if (close(deskryptor_fifo) == -1) {
        perror("Błąd zamykania FIFO");
        return -1;
    }
    return 0;
}

int usun_fifo(const char *sciezka_fifo) {
    if (unlink(sciezka_fifo) == -1) {
        perror("❌ Błąd usuwania FIFO");
        return -1;
    }
    return 0;
}
