#include "unp.h"
#include <sys/sem.h>
#include <sys/stat.h>

#define MAX_NIDS    4096
#define MAX_VALUE   (1024 * 1024)
#define MAX_MEMBERS 4096
#define MAX_NOPS    4096
#define MAX_NPROC   Sysconf(_SC_CHILD_MAX)

long Sysconf(int sc) {
    int save_errno = errno;
    errno = 0;
    long value = sysconf(sc);
    if (value == -1 && errno != 0) {
        err_sys("sysconf error");
    }
    errno = save_errno;
    return value;
}

int main(int argc, char **argv) {
    int i, j, semid, sid[MAX_NIDS], pipefd[2];
    int semmni, semvmx, semmsl, semmns, semopn, semaem, semume, semmnu;
    pid_t *child;
    union semun arg;
    struct sembuf ops[MAX_NOPS];

    // see how many sets with once member we can create
    for (i = 0; i <= MAX_NIDS; i++) {
        sid[i] = semget(IPC_PRIVATE, 1, SVSEM_MODE | IPC_CREAT);
        if (sid[i] == -1) {
            semmni = i;
            printf("%d identifiers open at once\n", semmni);
            break;
        }
    }
    for (j = 0; j < MAX_VALUE; j += 8) {
        arg.val = j;
        if (semctl(sid[0], 0, SETVAL, arg) == -1) {
            semvmx = j - 8;
            printf("max semaphore value = %d\n", semvmx);
            break;
        }
    }
    for (j = 0; j < i; j++) {
        if (semctl(sid[j], 0, IPC_RMID) == -1) {
            err_sys("semctl error");
        }
    }
    
    // determine max # semaphores per semaphore set
    for (i = 1; i <= MAX_MEMBERS; i++) {
        semid = semget(IPC_PRIVATE, i, SVSEM_MODE | IPC_CREAT);
        if (semid == -1) {
            semmsl = i - 1;
            printf("max of %d members per set\n", semmsl);
            break;
        }
        if (semctl(semid, 0, IPC_RMID) == -1) {
            err_sys("semctl error");
        }
    }

    // find max of total # of semaphores we can create
    semmns = 0;
    for (i = 0; i < semmni; i++) {
        sid[i] = semget(IPC_PRIVATE, semmsl, SVSEM_MODE | IPC_CREAT);
        if (sid[i] == -1) {
            /*
             * Up to this point each set has been created with semmsl
             * members. But this just failed, so try recreating this
             * final set with one fewer member per set, until it works.
             */
            for (j = semmsl - 1; j > 0; j--) {
                sid[i] = semget(IPC_PRIVATE, j, SVSEM_MODE | IPC_CREAT);
                if (sid[i] != -1) {
                    semmns += j;
                    printf("max of %d semaphores\n", semmns);
                    if (semctl(sid[i], 0, IPC_RMID) == -1) {
                        err_sys("semctl error");
                    }
                    goto done;
                }
            }
            err_quit("j reached 0, semmns = %d", semmns);
        }
        semmns += semmsl;
    }
    printf("max of %d semaphores\n", semmns);
done:
    for (j = 0; j < i; j++) {
        if (semctl(sid[j], 0, IPC_RMID) == -1) {
            err_sys("semctl error");
        }
    }

    // see how many operations per semop()
    if ((semid = semget(IPC_PRIVATE, semmsl, SVSEM_MODE | IPC_CREAT)) == -1) {
        err_sys("semget error");
    }
    for (i = 1; i <= MAX_NOPS; i++) {
        ops[i - 1].sem_num = i - 1;
        ops[i - 1].sem_op = 1;
        ops[i - 1].sem_flg = 0;
        if (semop(semid, ops, i) == -1) {
            if (errno != E2BIG) {
                err_sys("expected E2BIG from semop");
            }
            semopn = i - 1;
            printf("max of %d operations per semop()\n", semopn);
            break;
        }
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        err_sys("semctl error");
    }

    // determine the max value of semadj
    // create one set with one semaphore
    if ((semid = semget(IPC_PRIVATE, 1, SVSEM_MODE | IPC_CREAT)) == -1) {
        err_sys("semget error");
    }
    arg.val = semvmx;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        err_sys("semctl error");
    }
    for (i = semvmx - 1; i > 0; i--) {
        ops[0].sem_num = 0;
        ops[0].sem_op = -i;
        ops[0].sem_flg = SEM_UNDO;
        if (semop(semid, ops, 1) != -1) {
            semaem = i;
            printf("max value of adjust-on-exit = %d\n", semaem);
            break;
        }
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        err_sys("semctl error");
    }

    // determine max # undo structures
    // create one set with one semaphore; init to 0
    if ((semid = semget(IPC_PRIVATE, 1, SVSEM_MODE | IPC_CREAT)) == -1) {
        err_sys("semget error");
    }
    arg.val = 0;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        err_sys("semctl error");
    }
    if (pipe(pipefd) == -1) {
        err_sys("pipe error");
    }
    child = malloc(MAX_NPROC * sizeof(pid_t));
    for (i = 0; i < MAX_NPROC; i++) {
        if ((child[i] = fork()) == -1) {
            semmnu = i - 1;
            printf("fork failed, semmnu at least %d\n", semmnu);
            break;
        } else if (child[i] == 0) {
            ops[0].sem_num = 0;
            ops[0].sem_op = 1;
            ops[0].sem_flg = SEM_UNDO;
            j = semop(semid, ops, 1);
            if (write(pipefd[1], &j, sizeof(j)) == -1) {
                err_sys("write error");
            }
            sleep(30);
            exit(0);
        }
        if (read(pipefd[0], &j, sizeof(j)) == -1) {
            err_sys("read error");
        }
        if (j == -1) {
            semmnu = i;
            printf("max # undo structures = %d\n", semmnu);
            break;
        }
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        err_sys("semctl error");
    }
    for (j = 0; j <= i && child[j] > 0; j++) {
        kill(child[j], SIGINT);
    }

    // determine max # adjust entries per process
    // create one set with max # of semaphores
    if ((semid = semget(IPC_PRIVATE, semmsl, SVSEM_MODE | IPC_CREAT)) == -1) {
        err_sys("semget error");
    }
    for (i = 0; i < semmsl; i++) {
        arg.val = 0;
        if (semctl(semid, i, SETVAL, arg) == -1) {
            err_sys("semctl error");
        }
        ops[i].sem_num = i;
        ops[i].sem_op = 1;
        ops[i].sem_flg = SEM_UNDO;
        if (semop(semid, ops, i + 1) == -1) {
            semume = i;
            printf("max # undo entries per process = %d\n", semume);
            break;
        }
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        err_sys("semctl error");
    }
}
