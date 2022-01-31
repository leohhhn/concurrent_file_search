#include "helpers/threadHelpers.h"
#include "helpers/generalHelpers.h"

sem_t *workerSemaphore;
int numOfWorkers;
pthread_t *workerThreads; // malloced array of pthreads

void *workerFunction(void *args) {
    sem_wait(workerSemaphore);
    printf("From thread: %d\n", ((workerArgs *) args)->id);

}

void *mainMenu() {
    numOfWorkers = 0;
    printf("Unesite broj Worker niti: ");
    scanf("%d", &numOfWorkers);

    if ((workerSemaphore = sem_open("/workerSemaphore", O_CREAT, 0644, 5)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    workerThreads = (pthread_t *) malloc(numOfWorkers * sizeof(pthread_t));

    workerArgs wargs[numOfWorkers];

    for (int i = 0; i < numOfWorkers; i++) {
        wargs[i] = (workerArgs) {i, 1};
        printf("creating thread %d with args: id=%d avail=%d\n", i, wargs[i].id, wargs[i].available);
        pthread_create(workerThreads + i, NULL, workerFunction, wargs + i);

    }

    for (int i = 0; i < numOfWorkers; i++) {

//        pthread_join(*(workerThreads + i), NULL);
    }

    int i = 0;
    while (1) {


    }
}

int main(int argc, char *argv[]) {
    printf("Path relative to the working directory is: %s\n", argv[0]);

    pthread_t commandLineThread;
    pthread_create(&commandLineThread, NULL, mainMenu, NULL);

    pthread_join(commandLineThread, NULL);
    return 0;
}