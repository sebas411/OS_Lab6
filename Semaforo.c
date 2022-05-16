#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define THR_NUM 5
#define ITER_NUM 3
#define RESOURCES 3


sem_t sema;
FILE *logfile;

void work(int id) {
    struct timespec ts;

    for (int i = 0; i < ITER_NUM; i++) {
        fprintf(logfile, "%d - Iniciando iteracion %d\n", id, (i + 1));
        sem_wait(&sema);
        fprintf(logfile, "%d - Recurso tomado\n", id);
        int milisec = rand() % 3000;
        ts.tv_sec = milisec / 1000;
        ts.tv_nsec = (milisec % 1000) * 1000000;

        nanosleep(&ts, NULL);
        sem_post(&sema);
        fprintf(logfile, "%d - Recurso devuelto\n", id);
    }
}

void *thr_fun() {
    pid_t tid = syscall(SYS_gettid);
    fprintf(logfile, "Iniciando thread %d\n", tid);
    work(tid);
    fprintf(logfile, "Finalizando thread %d\n", tid);
    pthread_exit(0);
}

int main() {
    srand(time(0));
    logfile = fopen("./semaforo.log", "w");

    pthread_t threads[THR_NUM];
    fprintf(logfile, "Iniciando programa\n");
    fprintf(logfile, "Creando threads\n");

    sem_init(&sema, 0, RESOURCES);

    for (int i = 0; i < THR_NUM; i++) {
        pthread_create(&threads[i], NULL, thr_fun, 0);
    }
    fprintf(logfile, "Esperando threads\n");
    for (int i = 0; i < THR_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sema);
    fprintf(logfile, "Fin del programa\n");
    fclose(logfile);
    return (0);
}
