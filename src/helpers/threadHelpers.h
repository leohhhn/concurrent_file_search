#include <time.h>
#include <sys/stat.h>
#include <string.h>

int makeWatcher(char *parentPath, char *currPath, int *toTerminate) {
    char tmp[512];
    strcpy(tmp, parentPath); // parent path is always absolute, does not end with "/"
    strcat(tmp, "/");
    strcat(tmp, currPath);
    pthread_t *t = (pthread_t *) malloc(sizeof(pthread_t)); // todo where should i free this memory?

    watcherArgs *wa = malloc(sizeof(watcherArgs));

    wa->toTerminate = toTerminate;
    wa->root = 0;
    strcpy(wa->path, tmp);

    return !pthread_create(t, NULL, watcher, (void *) wa);
}


int folderIsNew(filesAndFolders *faf, char *currFolder) {
    for (int i = 0; i < faf->folderNum; i++)
        if (strcmp(faf->folders[i], currFolder) == 0)
            return 0; // folder already exists
    return 1;
}

//int fileModifiedOrNew(filesAndFolders *faf, char *currFilePath, char currTime[50]) {
//    for (int i = 0; i < faf->fileNum; i++) {
//        if (strcmp(faf->files[i].path, currFilePath) == 0) { // if file is not new
//            if (strcmp(faf->files[i].lastModified, currTime) != 0) // is the file modified?
//                return 1;
//            return 0;
//        }
//    }
//}
//
//int getLastModificationTime(char *currFilePath, char returnTime[50]) {
//
//    struct stat attrib;
//    stat(currFilePath, &attrib);
//    strftime(returnTime, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));
//}



