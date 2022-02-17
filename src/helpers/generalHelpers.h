#include <math.h>
#include <string.h>

/// Checks if @param input is prime
int isNumberPrime(int input) {
    if (input <= 1) return 0;
    for (int i = 2; i <= sqrt(input); i++)
        if (input % i == 0) return 0;
    return 1;
}

/// Searches for prime numbers given file pointer f
/// @param workerID - ID of worker thread
/// @note - args below are pointers to treeNode values, forwarded from the worker
/// @param currentRes - current result of node that is being worked on
/// @param partial - partially searched flag
void searchFileForPrimeNumbers(FILE *f, int *currentRes, int *partial) {
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

        // used to slow down workers
        usleep(100);
    }

    *partial = 0;
    *currentRes = numberOfPrimes;
    free(buffer);
    return;
}

/// Checks if a dir is at the path
int isDirectory(const char *path) {
    FILE *f = fopen(path, "r+");
    if (f) {
        fclose(f);
        return 0;
    }
    return 1;
}

/// Checks if a txt file is at the path
int isTxtFile(const char *path) {
    // check for txt extension
    if (strcmp(path + strlen(path) - 4, ".txt") == 0)
        return 1;
    return 0;
}

/// Concatenates two strings with a slash in between
/// @note concatenated path is stored in fullFilePath
void
buildPath(char parentPath[MAX_PATH_LENGTH], char currentName[MAX_NAME_LENGTH], char fullFilePath[MAX_PATH_LENGTH]) {
    strcpy(fullFilePath, parentPath);
    strcat(fullFilePath, "/");
    strcat(fullFilePath, currentName);
    return;
}

/// Checks if a folder is new
/// @param faf - watcher's faf
/// @param currFolder - name of folder to check for
int folderIsNew(filesAndFolders *faf, char *currFolder) {
    for (int i = 0; i < MAX_FOLDERS; i++)
        if (faf->folders[i] && strcmp(faf->folders[i], currFolder) == 0)
            return 0; // folder already exists
    return 1; // folder is new
}

/// Checks if a file is new or modified
/// @param faf - watcher's faf
/// @param currFileName - name of file to check for
/// @param modifiedTime - last modified time of file. calculated previously by watcher
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

/// Gets last modification time of a file
/// @param parentPath - path of the parent dir
/// @param currFileName - name of the file to check
/// @param returnTime - return variable for the last modification time
void getLastModificationTime(char parentPath[MAX_PATH_LENGTH], char *currFileName, char returnTime[50]) {
    char fullFilePath[MAX_PATH_LENGTH];
    buildPath(parentPath, currFileName, fullFilePath);

    struct stat attrib;
    stat(fullFilePath, &attrib);
    strftime(returnTime, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));
}

/// Prints current roots
void printCurrentRoots() {
    for (int i = 0; i < 64; ++i) {
        if (treeRoots[i])
            printTreeNode(treeRoots[i], 0);
    }
}






