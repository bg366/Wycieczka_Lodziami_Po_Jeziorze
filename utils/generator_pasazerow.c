#include "generator_pasazerow.h"
#include "interfejs.h"
#include "../pasazer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define MIN_SLEEP 1
#define MAX_SLEEP 1

/* Funkcja pomocnicza generująca losową liczbę z przedziału [min, max] */
static int losowy_okres_czasu(int min, int max)
{
    if (max <= min) return min;
    return rand() % (max - min + 1) + min;
}

/* Funkcja do obsługi sygnału – przerwanie pętli w generatorze */
static volatile sig_atomic_t stop_flaga = 0;
static void sig_handler(int signo)
{
    (void)signo; // ignoruj param
    stop_flaga = 1;
}

static volatile sig_atomic_t zatrzymane_lodzie_flaga = 0;
static void sig_handler_lodzie(int signo)
{
    (void)signo; // ignoruj param
    zatrzymane_lodzie_flaga++;
}

/* Kod wykonywany przez proces generatora: w pętli tworzy pasażerów w losowych odstępach czasu, aż do otrzymania sygnału. */
static void generuj_pasazerow(struct tm *godzina_zamkniecia)
{
    srand(time(NULL) ^ (getpid()<<16));

    printf(YELLOW "[GENERATOR_PASAZEROW %d] Start generatora pasażerów.\n" RESET, getpid());

    while (!stop_flaga && zatrzymane_lodzie_flaga != 2)
    {
        if (czy_minela_godzina(godzina_zamkniecia)) {
            printf(YELLOW "[GENERATOR_PASAZEROW %d] Minęła godzina. \n" RESET, getpid());
            break;
        }

        // Stwórz pasażera
        pid_t p = stworz_pasazera();
        if (p > 0)
        {
            printf(YELLOW "[GENERATOR_PASAZEROW %d] Utworzono pasażera PID=%d\n" RESET, getpid(), p);
        }

        // Odczekaj losowy czas
        int sl = losowy_okres_czasu(MIN_SLEEP, MAX_SLEEP);
        sleep(sl);
    }

    printf(YELLOW "[GENERATOR_PASAZEROW %d] Kończę pętlę generatora (otrzymano sygnał).\n" RESET, getpid());
}

/* Uruchamia nowy proces – generator pasażerów */
pid_t stworz_generator_pasazerow(struct tm *godzina_zamkniecia)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork() dla generatora");
        return -1;
    }
    if (pid == 0)
    {
        // Proces potomny
        // Obsługa sygnału do zakończenia
        signal(SIGTERM, sig_handler);
        signal(SIGUSR1, sig_handler_lodzie);
        signal(SIGUSR2, sig_handler_lodzie);

        generuj_pasazerow(godzina_zamkniecia);

        pid_t pid;
        while ((pid = wait(NULL)) > 0) {
            // printf("Child %d exited.\n", pid);
        }

        printf(YELLOW "[GENERATOR_PASAZEROW %d] Exit\n" RESET, getpid());

        _exit(0);
    }
    return pid;
}

/* Zatrzymuje proces generatora (wysyła sygnał + czeka na zakończenie) */
int zatrzymaj_generator_pasazerow(pid_t generatorPid)
{
    if (generatorPid <= 0)
    {
        fprintf(stderr, "Niepoprawny PID generatora!\n");
        return -1;
    }
    if (kill(generatorPid, SIGTERM) == -1)
    {
        if (errno == ESRCH)
        {
            // Proces już nie istnieje
            return 0;
        }
        perror("kill(generatorPid, SIGTERM)");
        return -1;
    }
    // Czekamy na zakończenie
    int status;
    if (waitpid(generatorPid, &status, 0) == -1)
    {
        perror("waitpid(generatorPid)");
        return -1;
    }

    return 0;
}
