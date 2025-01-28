#include "semafor.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>

int stworz_semafor(key_t klucz, int max_pomost_1, int max_pomost_2, int max_lodz_1, int max_lodz_2)
{
    int semid = semget(klucz, 4, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return -1;
    }

    union semun arg1;
    arg1.val = max_pomost_1;

    if (semctl(semid, 0, SETVAL, arg1) == -1) {
        perror("semctl(SETVAL)");
        return -1;
    }

    union semun arg2;
    arg2.val = max_pomost_2;

    if (semctl(semid, 1, SETVAL, arg2) == -1) {
        perror("semctl(SETVAL)");
        return -1;
    }

    union semun arg3;
    arg3.val = max_lodz_1;

    if (semctl(semid, 2, SETVAL, arg3) == -1) {
        perror("semctl(SETVAL)");
        return -1;
    }

    union semun arg4;
    arg4.val = max_lodz_2;

    if (semctl(semid, 3, SETVAL, arg4) == -1) {
        perror("semctl(SETVAL)");
        return -1;
    }
    return semid;
}

int podlacz_semafor(key_t klucz)
{
    int semid = semget(klucz, 1, 0666);
    if (semid == -1) {
        perror("semget");
        return -1;
    }

    return semid;
}

int usun_semafor(int semid)
{
    if (semid < 0) return -1;
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl(IPC_RMID)");
        return -1;
    }
    return 0;
}

int pobierz_wartosc_semafor(int semid, identyfikator_semaforu_t semafor)
{
    if (semid < 0) return -1;

    int val = semctl(semid, semafor, GETVAL, 0);
    if (val == -1) {
        perror("semctl(GETVAL)");
        return -1;
    }
    return val;
}

int opusc_semafor(int semid, identyfikator_semaforu_t semafor, short ilosc)
{
    if (semid < 0) return -1;

    struct sembuf sb;
    sb.sem_num = semafor;
    sb.sem_op = ilosc;
    sb.sem_flg = 0;   /* brak IPC_NOWAIT – jeśli 0, czekamy */

    if (semop(semid, &sb, 1) == -1) {
        if (errno != EINTR) {
            perror("semop(opusc)");
        }
        return -1;
    }

    return 0;
}

int podnies_semafor(int semid, identyfikator_semaforu_t semafor, short ilosc)
{
    if (semid < 0) return -1;

    struct sembuf sb;
    sb.sem_num = semafor;
    sb.sem_op = ilosc;
    sb.sem_flg = 0;

    if (semop(semid, &sb, 1) == -1) {
        perror("semop(podnies)");
        return -1;
    }

    return 0;
}
