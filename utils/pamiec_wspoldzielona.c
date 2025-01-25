#include "pamiec_wspoldzielona.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>

static void inicjuj_kolejke_fifo(kolejka_fifo_t *kolejka) {
    kolejka->poczatek = 0;
    kolejka->koniec   = 0;
    kolejka->liczba   = 0;
}

static int enqueue(kolejka_fifo_t *k, pid_t p) {
    if (k->liczba >= MAX_PASAZEROW) {
        return -1; /* kolejka pełna */
    }
    k->dane[k->koniec] = p;
    k->koniec = (k->koniec + 1) % MAX_PASAZEROW;
    k->liczba++;
    return 0;
}

static int dequeue(kolejka_fifo_t *k, pid_t *out) {
    if (k->liczba == 0) {
        return -1; /* kolejka pusta */
    }
    if (out != NULL) {
        *out = k->dane[k->poczatek];
    }
    k->poczatek = (k->poczatek + 1) % MAX_PASAZEROW;
    k->liczba--;
    return 0;
}

int stworz_pamiec_wspoldzielona(key_t klucz) {
    size_t rozmiar = sizeof(dane_wspolne_t);
    int shmid = shmget(klucz, rozmiar, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
    }

    return shmid;
}

dane_wspolne_t* odbierz_dane_wspolne(int shmid) {
    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void*)-1) {
        perror("shmat");
        return NULL;
    }
    return (dane_wspolne_t*)addr;
}

dane_wspolne_t* dolacz_pamiec_wspoldzielona(key_t klucz) {
    size_t rozmiar = sizeof(dane_wspolne_t);
    int shmid = shmget(klucz, rozmiar, 0666);
    if (shmid < 0) {
        perror("shmget");
        return NULL;
    }

    void *addr = shmat(shmid, NULL, 0);
    if (addr == (void*)-1) {
        perror("shmat");
        return NULL;
    }
    return (dane_wspolne_t*)addr;
}

int inicjuj_dane_wspolne(dane_wspolne_t *dw) {
    if (!dw) return -1;

    /* Inicjalizujemy kolejki */
    inicjuj_kolejke_fifo(&dw->kolejka_1_normalna);
    inicjuj_kolejke_fifo(&dw->kolejka_1_vip);
    inicjuj_kolejke_fifo(&dw->kolejka_2_normalna);
    inicjuj_kolejke_fifo(&dw->kolejka_2_vip);
    inicjuj_kolejke_fifo(&dw->lodz_1);
    inicjuj_kolejke_fifo(&dw->lodz_2);
    dw->jest_koniec = 0;

    /* Inicjalizacja mutexów między-procesowych */
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
        perror("pthread_mutexattr_init");
        return -1;
    }

    /* Ustawiamy, by mutex był współdzielony między różnymi procesami */
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED) != 0) {
        perror("pthread_mutexattr_setpshared");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }

    if (pthread_mutex_init(&dw->mutex_kolejka_1_normalna, &attr) != 0) {
        perror("pthread_mutex_init(kolejka_1_normalna)");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    if (pthread_mutex_init(&dw->mutex_kolejka_1_vip, &attr) != 0) {
        perror("pthread_mutex_init(kolejka_1_vip)");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    if (pthread_mutex_init(&dw->mutex_kolejka_2_normalna, &attr) != 0) {
        perror("pthread_mutex_init(kolejka_2_normalna)");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    if (pthread_mutex_init(&dw->mutex_kolejka_2_vip, &attr) != 0) {
        perror("pthread_mutex_init(kolejka_2_vip)");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    if (pthread_mutex_init(&dw->mutex_lodz_1, &attr) != 0) {
        perror("pthread_mutex_init(lodz_1)");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }
    if (pthread_mutex_init(&dw->mutex_lodz_2, &attr) != 0) {
        perror("pthread_mutex_init(lodz_2)");
        pthread_mutexattr_destroy(&attr);
        return -1;
    }

    /* Usuwamy atrybuty, mutexy już zainicjalizowane. */
    pthread_mutexattr_destroy(&attr);

    return 0;
}

static int wybierz_kolejke_i_mutex(dane_wspolne_t *dw,
                                   identyfikator_kolejki_t gdzie,
                                   kolejka_fifo_t **out_kolejka,
                                   pthread_mutex_t **out_mutex)
{
    if (!dw) return -1;
    switch (gdzie) {
        case KOLEJKA_1_NORMALNA:
            *out_kolejka = &dw->kolejka_1_normalna;
            *out_mutex   = &dw->mutex_kolejka_1_normalna;
            return 0;
        case KOLEJKA_1_VIP:
            *out_kolejka = &dw->kolejka_1_vip;
            *out_mutex   = &dw->mutex_kolejka_1_vip;
            return 0;
        case KOLEJKA_2_NORMALNA:
            *out_kolejka = &dw->kolejka_2_normalna;
            *out_mutex   = &dw->mutex_kolejka_2_normalna;
            return 0;
        case KOLEJKA_2_VIP:
            *out_kolejka = &dw->kolejka_2_vip;
            *out_mutex   = &dw->mutex_kolejka_2_vip;
            return 0;
        case LODZ_1:
            *out_kolejka = &dw->lodz_1;
            *out_mutex   = &dw->mutex_lodz_1;
            return 0;
        case LODZ_2:
            *out_kolejka = &dw->lodz_2;
            *out_mutex   = &dw->mutex_lodz_2;
            return 0;
        default:
            return -1;
    }
}

int dodaj_pasazera(dane_wspolne_t *dw, identyfikator_kolejki_t gdzie, pid_t pasazer) {
    kolejka_fifo_t *k = NULL;
    pthread_mutex_t *mtx = NULL;

    if (wybierz_kolejke_i_mutex(dw, gdzie, &k, &mtx) != 0) {
        fprintf(stderr, "Nieznany identyfikator kolejki/łodzi: %d\n", gdzie);
        return -1;
    }

    /* Blokujemy dostęp do kolejki */
    pthread_mutex_lock(mtx);
    int wynik = enqueue(k, pasazer);
    pthread_mutex_unlock(mtx);

    return wynik;
}

int zdejmij_pasazera(dane_wspolne_t *dw, identyfikator_kolejki_t gdzie, pid_t *wynik) {
    kolejka_fifo_t *k = NULL;
    pthread_mutex_t *mtx = NULL;

    if (wybierz_kolejke_i_mutex(dw, gdzie, &k, &mtx) != 0) {
        fprintf(stderr, "Nieznany identyfikator kolejki/łodzi: %d\n", gdzie);
        return -1;
    }

    pthread_mutex_lock(mtx);
    int kod = dequeue(k, wynik);
    pthread_mutex_unlock(mtx);

    return kod;
}

int pobierz_liczbe_pasazerow(dane_wspolne_t *dw, identyfikator_kolejki_t gdzie) {
    kolejka_fifo_t *k = NULL;
    pthread_mutex_t *mtx = NULL;

    if (wybierz_kolejke_i_mutex(dw, gdzie, &k, &mtx) != 0) {
        return -1;
    }

    pthread_mutex_lock(mtx);
    int liczba = k->liczba;
    pthread_mutex_unlock(mtx);

    return liczba;
}

int zakoncz_pamiec_wspoldzielona(dane_wspolne_t *dw, int shmid) {
    if (!dw) return -1;

    /* Niszczymy mutexy */
    pthread_mutex_destroy(&dw->mutex_kolejka_1_normalna);
    pthread_mutex_destroy(&dw->mutex_kolejka_1_vip);
    pthread_mutex_destroy(&dw->mutex_kolejka_2_normalna);
    pthread_mutex_destroy(&dw->mutex_kolejka_2_vip);
    pthread_mutex_destroy(&dw->mutex_lodz_1);
    pthread_mutex_destroy(&dw->mutex_lodz_2);

    /* Odłączamy się od pamięci */
    if (shmdt((void*)dw) == -1) {
        perror("shmdt");
        return -1;
    }

    /* Jeżeli chcemy usunąć segment z systemu (IPC_RMID) */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl(IPC_RMID)");
        return -1;
    }

    return 0;
}
