#include "policjant.h"
#include "utils/interfejs.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void logika_policjanta() {
    printf(RED "[POLICJANT %d] Rozpoczynam logikę policjanta...\n" RESET, getpid());

    printf(RED "[POLICJANT %d] Kończę.\n" RESET, getpid());
    _exit(0); // Bezpieczne zakończenie procesu potomnego
}

pid_t stworz_policjanta() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork() dla pasażera");
        return -1;
    }
    if (pid == 0) {
        // Proces potomny
        logika_policjanta();
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
