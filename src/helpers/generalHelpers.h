#include <math.h>
#include <string.h>

/// Returns 1 if number is prime, 0 otherwise
int isNumberPrime(int input) {
    if (input <= 1) return 0;
    for (int i = 2; i <= sqrt(input); i++)
        if (input % i == 0) return 0;
    return 1;
}

void searchFileForNumbers(FILE *f, int *currentRes, int workerID, int *partial, char *fileName) {

    int numberOfPrimes = 0, searchedSoFar = 0, i = 0;
    char *buffer = malloc(BLOCK_SIZE * sizeof(char));

    while (fgets(buffer, BLOCK_SIZE * sizeof(char), f)) {
        searchedSoFar += (int) strlen(buffer);
        if (searchedSoFar / BLOCK_SIZE > i) {
            *currentRes = numberOfPrimes;
            i++;
        }

        char *tmp = buffer;
        while (*tmp) {
            if (isdigit(*tmp)) {
                int val = (int) strtol(tmp, &tmp, 10);
                if (isNumberPrime(val))
                    numberOfPrimes++;
            } else {
                tmp++;
            }
        }
    }

    *partial = 0;
    *currentRes = numberOfPrimes;
    free(buffer);
    return;
}

int isDirectory(const char *path) {
    FILE *f = fopen(path, "r+");
    if (f) {
        fclose(f);
        return 0;
    }
    return 1;
}

int isTxtFile(const char *path) {
    // check for txt extension
    if (strcmp(path + strlen(path) - 4, ".txt") == 0)
        return 1;
    return 0;
}

void
buildPath(char parentPath[MAX_PATH_LENGTH], char currentName[MAX_NAME_LENGTH], char fullFilePath[MAX_PATH_LENGTH]) {
    strcpy(fullFilePath, parentPath);
    strcat(fullFilePath, "/");
    strcat(fullFilePath, currentName);
    return;
}

int folderIsNew(filesAndFolders *faf, char *currFolder) {
    for (int i = 0; i < MAX_FOLDERS; i++)
        if (faf->folders[i] && strcmp(faf->folders[i], currFolder) == 0)
            return 0; // folder already exists
    return 1; // folder is new
}

int fileIsNewOrModified(filesAndFolders *faf, char *currFileName, char modifiedTime[50]) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (faf->files[i]) {
            if (strcmp(faf->files[i]->name, currFileName) == 0) {
                if (strcmp(faf->files[i]->lastModified, modifiedTime) != 0) {
                    strcpy(faf->files[i]->lastModified, modifiedTime);
                    return 2; // modified
                } else
                    return 0; // file is not new and was not modified
            }
        }
    }
    return 1; // file is new
}

void getLastModificationTime(char parentPath[MAX_PATH_LENGTH], char *currFileName, char returnTime[50]) {
    char fullFilePath[MAX_PATH_LENGTH];
    buildPath(parentPath, currFileName, fullFilePath);

    struct stat attrib;
    stat(fullFilePath, &attrib);
    strftime(returnTime, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));
}

void printCurrentRoots() {
    for (int i = 0; i < 64; ++i) {
        if (treeRoots[i])
            printTreeNode(treeRoots[i], 0);
    }
}






