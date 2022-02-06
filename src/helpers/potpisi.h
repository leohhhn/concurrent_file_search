#ifndef CONCURRENT_FILE_SEARCH_POTPISI_H
#define CONCURRENT_FILE_SEARCH_POTPISI_H

#endif //CONCURRENT_FILE_SEARCH_POTPISI_H

#define BLOCK_SIZE 1024
#define MAX_FOLDERS 64
#define MAX_FILES MAX_FOLDERS
#define MAX_PATH_LENGTH 512
#define MAX_NAME_LENGTH 128

#define WATCHER_SLEEP_TIME 2

typedef struct workerArgs {
    int id;
    int available; // 1 if available, 0 if working
} workerArgs;

typedef struct watcherArgs {
    int *toTerminate;
    int root; // 1 if watcher is root
    char path[MAX_PATH_LENGTH];
    pthread_t *self;
} watcherArgs;

typedef struct fileInfo {
    char *name; // filename without paths before
    char *lastModified; // returned by stat
} fileInfo;

typedef struct filesAndFolders {
    fileInfo *files; // array of all non-dir files in watched dir
    char **folders;
    int fileNum;
    int folderNum;
} filesAndFolders;

int isNumberPrime(int input);

void helpMenu();

int makeWatcher(char *parentPath, char *currPath, int *toTerminate);

int folderIsNew(filesAndFolders *faf, char *currFolder);

int fileIsNewOrModified(filesAndFolders *faf, char *currFileName, char modfTime[50]);

void getLastModificationTime(char parentPath[MAX_PATH_LENGTH], char *currFileName, char returnTime[50]);

int scanDir(char path[512], filesAndFolders *faf, int *toTerminate);

int scanFile(char path[MAX_NAME_LENGTH], int *done, int *currRes, const int *workerID);

void *watcher(void *_args);

void *worker(void *_args);
