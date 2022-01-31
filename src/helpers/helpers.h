#include <pthread.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "potpisi.h"

int scanFile(char path[512]) {

    FILE *f = fopen(path, "r");

    if (f) {
        printf("Opened file at %s\n", path);
        int numberOfPrimes = 0, searchedSoFar = 0;
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

    } else {
        printf("Failed to open file at %s", path);
    }
    return 0;
}

/// Returns 1 if number is prime, 0 otherwise
int isNumberPrime(int input) {

    if (input <= 1) return 0;
    for (int i = 2; i <= sqrt(input); i++)
        if (input % i == 0) return 0;
    // printf("%d\n", input);
    return 1;
}





