#include "pasazer.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_pasazera() {
    /*
     * Logika pasażera:
     *  - Podłączenie się do pamięci wspólnej
     *  - Komunikacja z kasjerem (np. przez kolejkę komunikatów?)
     *  - Wejście na pomost (semafor)
     *  - Wejście na łódź, rejs, wyjście z łodzi, itp.
     */

    printf("[PASAZER %d] Rozpoczynam logikę pasażera...\n", getpid());
    sleep(1); // Symulacja krótkiej pracy
    printf("[PASAZER %d] Kończę.\n", getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

pid_t stworz_pasazera() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork() dla pasażera");
        return -1;
    }
    if (pid == 0) {
        // Proces potomny
        logika_pasazera();
        // Nie powinno się tu dotrzeć, bo logika_pasazera kończy proces
        _exit(0);
    }
    // Zwracamy PID dziecka
    return pid;
}

/* Funkcja do "zabicia" pasażera np. gdybyśmy chcieli przedwcześnie przerwać jego działanie */
int zatrzymaj_pasazera(pid_t passengerPid) {
    if (kill(passengerPid, SIGTERM) == -1) {
        perror("kill(passengerPid)");
        return -1;
    }
    // Poczekajmy, aż się zakończy
    int status;
    waitpid(passengerPid, &status, 0);
    return 0;
}
