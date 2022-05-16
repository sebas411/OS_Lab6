#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define THR_NUM 5
#define ITER_NUM 3
#define RESOURCES_CONSUMED 21
#define TOTAL_RESOURCES 80


pthread_mutex_t lock;
FILE *logfile;
int available_resources = TOTAL_RESOURCES;

/* decrease available resources by count resources */ 
/* return 0 if sufficient resources available, */ 
/* otherwise return -1 */ 
int decrease_count(int count) { 
  if (available_resources < count) 
    return -1; 
  else { 
    available_resources -= count; 
    return 0; 
  } 
} 
 
/* increase available resources by count */ 
int increase_count(int count) { 
  available_resources += count; 
  return 0; 
}

void work(int id) {
    struct timespec ts;
    int hasResources = 0;

    for (int i = 0; i < ITER_NUM; i++) {
        fprintf(logfile, "%d - Iniciando iteracion %d\n", id, (i + 1));
        while (1) {
          pthread_mutex_lock(&lock);
          if (decrease_count(RESOURCES_CONSUMED) == 0) hasResources = 1;
          pthread_mutex_unlock(&lock);
          if (hasResources) break;
          fprintf(logfile, "%d - No hay suficientes recursos (%d) esperando a que se liberen...\n", id, available_resources);
          sleep(1);
        }
        fprintf(logfile, "%d - Se consumen %d recursos\n", id, RESOURCES_CONSUMED);
        int milisec = rand() % 3000;
        ts.tv_sec = milisec / 1000;
        ts.tv_nsec = (milisec % 1000) * 1000000;
        nanosleep(&ts, NULL);

        pthread_mutex_lock(&lock);
        increase_count(RESOURCES_CONSUMED);
        pthread_mutex_unlock(&lock);
        fprintf(logfile, "%d - %d recursos devueltos\n", id, RESOURCES_CONSUMED);
        hasResources = 0;
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
    logfile = fopen("./monitor.log", "w");

    pthread_t threads[THR_NUM];
    fprintf(logfile, "Iniciando programa\n");
    fprintf(logfile, "Creando threads\n");

    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < THR_NUM; i++) {
        pthread_create(&threads[i], NULL, thr_fun, 0);
    }
    fprintf(logfile, "Esperando threads\n");
    for (int i = 0; i < THR_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    fprintf(logfile, "Fin del programa\n");
    fclose(logfile);
    return (0);
}
