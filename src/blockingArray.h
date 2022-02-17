#include <semaphore.h>
#include "declarations.h"

/// Adds treeNode to the Blocking Array
/// @note this function is called by a watcher
treeNode *addToBA(treeNode *nodeToAdd) {
    sem_wait(&empty);
    sem_wait(&blockingArrayMutex);

    blockingArray[indexIn] = nodeToAdd;
    indexIn = (indexIn + 1) % BA_SIZE;

    sem_post(&blockingArrayMutex);
    sem_post(&full);

    return nodeToAdd;
}

/// Reads a treeNode from the Blocking Array
/// @note this function is called by a worker
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