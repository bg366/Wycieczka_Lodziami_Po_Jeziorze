#include "policjant.h"
#include "utils/interfejs.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

#define SZANSA_ZATRZYMANIA 100

typedef struct {
    int numer_lodzi;
    time_t czas_konca;
} policjant_arg_t;

/* Funkcja wątku policjanta */
void* watek_policjanta(void *arg) {

    policjant_arg_t *p = (policjant_arg_t*)arg;
    time_t teraz = time(NULL);

    /* Obliczamy ile sekund zostało do p->czas_konca */
    double roznica = difftime(p->czas_konca, teraz) - 5;
    if (roznica <= 0) {
        /* Jeśli już po czasie docelowym, nie ma co robić. */
        printf(RED "[POLICJANT %lu] Czas docelowy już minął, wątek łodzi nr %d nic nie robi.\n" RESET,
               (unsigned long)pthread_self(), p->numer_lodzi);
        free(p);
        pthread_exit(NULL);
    }

    srand((unsigned)time(NULL) ^ ((uintptr_t)pthread_self() << 16));

    /* Losujemy, czy ten wątek zatrzyma łódź z 10% szansą */
    int r = rand() % 100;
    if (r < SZANSA_ZATRZYMANIA) {
        int kiedy = 5 + (rand() % ((int)roznica));
        printf(RED "[POLICJANT %lu] Wątek łodzi nr %d chce zatrzymać łódź za %d sekund.\n" RESET,
               (unsigned long)pthread_self(), p->numer_lodzi, kiedy);

        sleep(kiedy);

        /* Wysyłamy sygnał do łodzi – tu przykładowo do grupy procesów */
        if (p->numer_lodzi == 1) {
            printf(RED "[POLICJANT %lu] -> Wysyłam SIGUSR1 (zatrzymanie łodzi 1)\n" RESET,
                   (unsigned long)pthread_self());
            kill(0, SIGUSR1);
        } else {
            printf(RED "[POLICJANT %lu] -> Wysyłam SIGUSR2 (zatrzymanie łodzi 2)\n" RESET,
                   (unsigned long)pthread_self());
            kill(0, SIGUSR2);
        }
    } else {
        /* W 90% przypadków wątek nie zatrzymuje łodzi. */
        printf(RED "[POLICJANT %lu] Wątek łodzi nr %d nie zatrzymuje łodzi.\n" RESET,
               (unsigned long)pthread_self(), p->numer_lodzi);
    }

    free(p);
    pthread_exit(NULL);
}

void logika_policjanta(struct tm *godzina_zamkniecia) {
    printf(RED "[POLICJANT %d] Rozpoczynam logikę policjanta...\n" RESET, getpid());

    /* Konwertujemy struct tm -> time_t (sekundy od epoki) */
    time_t czas_docelowy = mktime(godzina_zamkniecia);
    if (czas_docelowy == (time_t)(-1)) {
        fprintf(stderr, "Błąd konwersji mktime().\n");
        return;
    }

    /* Sprawdzamy, ile czasu zostało (może być ujemne, jeśli data już minęła) */
    time_t teraz = time(NULL);
    double roznica = difftime(czas_docelowy, teraz);
    if (roznica <= 0) {
        printf("[MAIN] Podana data/godzina już minęła (lub jest za wczesna).\n");
        /* Możemy zdecydować, co zrobić – np. zakończyć program. */
        return;
    }

    printf("[MAIN] Do osiągnięcia docelowej godziny pozostało ok. %.0f sekund.\n", roznica);

    /* Tworzymy 2 wątki – każdy odpowiada za inną łódź. */
    pthread_t tid1, tid2;

    /* Alokujemy arg dla wątku 1 */
    policjant_arg_t *arg1 = (policjant_arg_t*)malloc(sizeof(policjant_arg_t));
    if (!arg1) { perror("malloc arg1"); return; }
    arg1->numer_lodzi = 1;
    arg1->czas_konca  = czas_docelowy;

    /* Alokujemy arg dla wątku 2 */
    policjant_arg_t *arg2 = (policjant_arg_t*)malloc(sizeof(policjant_arg_t));
    if (!arg2) { perror("malloc arg2"); free(arg1); return; }
    arg2->numer_lodzi = 2;
    arg2->czas_konca  = czas_docelowy;

    /* Uruchamiamy wątki */
    pthread_create(&tid1, NULL, watek_policjanta, arg1);
    pthread_create(&tid2, NULL, watek_policjanta, arg2);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    printf(RED "[POLICJANT %d] Kończę.\n" RESET, getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}


pid_t stworz_policjanta(struct tm *godzina_zamkniecia) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork() dla pasażera");
        return -1;
    }
    if (pid == 0) {
        // Proces potomny
        logika_policjanta(godzina_zamkniecia);
        _exit(0);
    }
    return pid;
}

/* Funkcja do "zabicia" policjanta np. gdybyśmy chcieli przedwcześnie przerwać jego działanie */
int zatrzymaj_policjanta(pid_t policjant_pid) {
    if (kill(policjant_pid, SIGTERM) == -1) {
        perror("kill(policjant_pid)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(policjant_pid, &status, 0);
    return 0;
}
