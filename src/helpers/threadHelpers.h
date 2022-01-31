#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "potpisi.h"

typedef struct workerArgs {
    int id;
    int available; // 1 if available, 0 if working
} workerArgs;

int scanFile(char path[512]) {

    FILE *f = fopen(path, "r");

    if (f) {
        fseek(f, 0, SEEK_END); // seek to end of file
        long fileSize = ftell(f); // get current file pointer
        fseek(f, 0, SEEK_SET); // seek back to beginning of file

        printf("file size: %ld\n", fileSize);
        printf("Opened file at %s\n", path);
        unsigned int numberOfPrimes = 0, searchedSoFar = 0;
        char *buffer = (char *) malloc(BLOCK_SIZE * sizeof(char));

        while (fgets(buffer, BLOCK_SIZE * sizeof(char), f) != 0) {
            searchedSoFar += strlen(buffer);
            while (*buffer) {
                if (isdigit(*buffer)) {
                    int val = (int) strtol(buffer, &buffer, 10);
                    if (isNumberPrime(val))
                        numberOfPrimes++;
                } else {
                    buffer++;
                }
            }
        }

        printf("no of prime no: %d\nLength of file: %d\n", numberOfPrimes, searchedSoFar);

        // free(buffer); todo see why error occurs here
    } else {
        printf("Failed to open file at %s", path);
    }
    return 0;
}

