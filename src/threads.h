#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "helpers/potpisi.h"
#include "helpers/threadHelpers.h"

// initial values: done = 0, currRes = 0
int scanFile(char path[MAX_NAME_LENGTH], int *done, int *currRes, const int *workerID) {
    FILE *f = fopen(path, "r");
    if (f) {
        printf("Opened file at %s\n", path);

//        // how big is the file?
//        fseek(f, 0, SEEK_END); // seek to end of file
//        long fileSize = ftell(f); // get current file pointer
//        fseek(f, 0, SEEK_SET); // seek back to beginning of file
//        int fileSizeInBlocks = ((int) fileSize) / BLOCK_SIZE;
//        int leftover = ((int) fileSize) % BLOCK_SIZE;

        int numberOfPrimes = 0, searchedSoFar = 0, i = 0;
        char *buffer = (char *) malloc(BLOCK_SIZE * sizeof(char));

        while (fgets(buffer, BLOCK_SIZE * sizeof(char), f) != 0) {
            searchedSoFar += (int) strlen(buffer);
            if (searchedSoFar / BLOCK_SIZE > i++) {
                // update res every BLOCK_SIZE
                *currRes = numberOfPrimes;
                printf("Worker %d searched %d bytes, and found %d prime numbers so far.\n", *workerID,
                       searchedSoFar, *currRes);
            }
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

        *done = 1;
        *currRes = numberOfPrimes;
        fclose(f);
        // free(buffer); todo see why error occurs here
    } else {
        printf("Failed to open file at %s\n", path);
        return 1; // err
    }
    return 0;
}

int scanDir(char path[512], filesAndFolders *faf, int *toTerminate) {
    // printf("Scanning dir at %s\n", path);
    DIR *folder = opendir(path);
    struct dirent *entry;

    if (folder == NULL) return -1;

    while ((entry = readdir(folder))) {
        // will break if fileNum or folderNum > MAX_FILE
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {
            // if you found a dir inside the dir, excluding . & ..
            if (!folderIsNew(faf, entry->d_name)) {// todo check if addressing is correct
                printf("nothing new, skipping\n");
                continue;
            }

            int watcherCreated = makeWatcher(path, entry->d_name, toTerminate);
            if (!watcherCreated) {
                printf("Failed to make child watcher.\n");
                continue;
            }

            strcpy(faf->folders[faf->folderNum++], entry->d_name); // copy folder name into folder array inside faf
            printf("Found dir %d: %s\n", faf->folderNum - 1, faf->folders[faf->folderNum - 1]);
        } else if (entry->d_type == DT_REG) {
            // if you found a file
            // todo add if file modified / new

//            if (!fileModifiedOrNew(faf, entry->d_name)) {
//                // todo remove printf after testing
//                printf("File %s was not modified and is not new, skipping.\n", entry->d_name);
//                continue;
//            }

//            strcpy(faf->files[faf->fileNum].path, entry->d_name); // copy file name into file array inside faf
//            printf("Dir %d: %s\n", faf->fileNum - 1, faf->folders[faf->fileNum - 1]);

            // todo blocking array things for workers here
        }
    }
    return 0;
}



