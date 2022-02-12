#include <semaphore.h>
#include "declarations.h"

void addToBA(char *fullFilePath, char **ba, sem_t *empty, sem_t *full, sem_t *baLock, int *in) {
    int writeIndex = *in;

    sem_wait(empty);
    sem_wait(baLock);



    sem_post(baLock);
    sem_post(full);
}

void readFromBA(char **ba, sem_t *empty, sem_t *full, sem_t *baLock, int *out) {
    int readIndex = *out;

    sem_wait(full);
    sem_wait(baLock);


    readIndex = (readIndex + 1) % BA_SIZE;

    sem_post(baLock);
    sem_post(empty);
}