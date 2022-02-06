#include <time.h>
#include <sys/stat.h>
#include <string.h>

int makeWatcher(char *parentPath, char *currPath, int *toTerminate) {
    char tmp[512];
    strcpy(tmp, parentPath); // parent name is always absolute, does not end with "/"
    strcat(tmp, "/");
    strcat(tmp, currPath);
    pthread_t *t = (pthread_t *) malloc(sizeof(pthread_t));

    watcherArgs *wa = malloc(sizeof(watcherArgs));

    wa->toTerminate = toTerminate;
    wa->root = 0;
    strcpy(wa->path, tmp);
    wa->self = t;

    return !pthread_create(t, NULL, watcher, (void *) wa);
}

int folderIsNew(filesAndFolders *faf, char *currFolder) {
    for (int i = 0; i < faf->folderNum; i++)
        if (strcmp(faf->folders[i], currFolder) == 0)
            return 0; // folder already exists
    return 1;
}

int fileIsNewOrModified(filesAndFolders *faf, char *currFileName, char modifiedTime[50]) {
    for (int i = 0; i < faf->fileNum; i++) {
        if (strcmp(faf->files[i].name, currFileName) == 0) {
            if (strcmp(faf->files[i].lastModified, modifiedTime) != 0) {
                strcpy(faf->files[i].lastModified, modifiedTime); // update modified time, easier here
                return 2; // was modified
            }
            return 0; // file is not new and was not modified
        }
    }
    return 1; // file is new
}

void getLastModificationTime(char parentPath[MAX_PATH_LENGTH], char *currFileName, char returnTime[50]) {
    // build name
    char fullFilePath[MAX_PATH_LENGTH];
    strcpy(fullFilePath, parentPath);
    strcat(fullFilePath, "/");
    strcat(fullFilePath, currFileName);

    struct stat attrib;
    stat(fullFilePath, &attrib);
    strftime(returnTime, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));

}



