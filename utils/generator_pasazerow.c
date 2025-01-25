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
#define MAX_SLEEP 3

/* Funkcja pomocnicza generująca losową liczbę z przedziału [min, max] */
static int losowy_okres_czasu(int min, int max)
{
    if (max <= min) return min;
    return rand() % (max - min + 1) + min;
}

/* Funkcja do obsługi sygnału – przerwanie pętli w generatorze */
static volatile sig_atomic_t stopFlag = 0;
static void sigHandler(int signo)
{
    (void)signo; // ignoruj param
    stopFlag = 1;
}

/* Kod wykonywany przez proces generatora: w pętli tworzy pasażerów w losowych odstępach czasu, aż do otrzymania sygnału. */
static void generuj_pasazerow()
{
    srand(time(NULL) ^ (getpid()<<16));

    printf(YELLOW "[GENERATOR_PASAZEROW %d] Start generatora pasażerów.\n" RESET, getpid());

    while (!stopFlag)
    {
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
    _exit(0);
}

/* Uruchamia nowy proces – generator pasażerów */
pid_t stworz_generator_pasazerow()
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
        signal(SIGTERM, sigHandler);

        generuj_pasazerow();

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
    printf(YELLOW "[GENERATOR_PASAZEROW PID=%d] Zakończył działanie.\n" RESET, generatorPid);

    return 0;
}
