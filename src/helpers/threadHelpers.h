#include <time.h>
#include <sys/stat.h>
#include <string.h>

// initArgs - not null only if creating root watcher
int makeWatcher(char *currPath, int *toTerminate, treeNode *parent, watcherArgs *initArgs) {
    pthread_t *t = malloc(sizeof(pthread_t));

    if (initArgs) {
        return !pthread_create(t, NULL, watcher, (void *) initArgs);
    } else {
        watcherArgs *wa = malloc(sizeof(watcherArgs));

        wa->path = malloc(sizeof(char) * MAX_PATH_LENGTH);
        wa->toTerminate = toTerminate;
        wa->self = t;
        wa->root = 0;
        wa->dirNode = addChildToParent(parent, currPath, 0);
        buildPath(parent->fullPath, currPath, wa->path);

        return !pthread_create(t, NULL, watcher, (void *) wa);
    }
}

int folderIsNew(filesAndFolders *faf, char *currFolder) {
    for (int i = 0; i < MAX_FOLDERS; i++)
        if (faf->folders[i] && strcmp(faf->folders[i], currFolder) == 0)
            return 0; // folder already exists
    return 1;
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



