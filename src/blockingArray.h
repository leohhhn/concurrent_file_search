#include <semaphore.h>
#include "declarations.h"

treeNode *addToBA(treeNode *nodeToAdd) {
    sem_wait(&empty);
    sem_wait(&blockingArrayMutex);

    blockingArray[indexIn] = nodeToAdd;
    indexIn = (indexIn + 1) % BA_SIZE;

    sem_post(&blockingArrayMutex);
    sem_post(&full);

    return nodeToAdd;
}

treeNode *readFromBA() {
    treeNode *ret;
    sem_wait(&full);
    sem_wait(&blockingArrayMutex);

    ret = blockingArray[indexOut];
    indexOut = (indexOut + 1) % BA_SIZE;

    sem_post(&blockingArrayMutex);
    sem_post(&empty);

    return ret;
}