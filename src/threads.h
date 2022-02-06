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
    // printf("Scanning dir at %s\n", name);
    DIR *folder = opendir(path);
    struct dirent *entry;

    if (folder == NULL) return -1;

    while ((entry = readdir(folder))) {
        // will break if fileNum or folderNum > MAX_FILE
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            if (!folderIsNew(faf, entry->d_name))
                continue;

            int watcherCreated = makeWatcher(path, entry->d_name, toTerminate);
            if (!watcherCreated)
                continue;

            strcpy(faf->folders[faf->folderNum++], entry->d_name); // copy folder name into folder array inside faf
            //printf("Found dir %d: %s\n", faf->folderNum - 1, faf->folders[faf->folderNum - 1]);
        } else if (entry->d_type == DT_REG) {
            char modifiedTime[50];
            getLastModificationTime(path, entry->d_name, modifiedTime);

            int modifySwitch = fileIsNewOrModified(faf, entry->d_name, modifiedTime);

            if (modifySwitch == 0) {
                continue;
            } else if (modifySwitch == 1) {
                strcpy(faf->files[faf->fileNum].name, entry->d_name);
                strcpy(faf->files[faf->fileNum++].lastModified, modifiedTime);
                printf("\nNew file found!\nName: %s\nLast modified: %s\ncurrent modified time: %s\n\n",
                       faf->files[faf->fileNum - 1].name,
                       faf->files[faf->fileNum - 1].lastModified, modifiedTime);
                // todo add file to blocking array
            } else if (modifySwitch == 2) {
                printf("\nFile was modified! \nName: %s\nNew modified time: %s\n\n",
                       faf->files[faf->fileNum - 1].name, modifiedTime);
                // todo add file to blocking array
            }
        }
    }
    return 0;
}



