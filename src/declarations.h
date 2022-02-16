#ifndef CONCURRENT_FILE_SEARCH_POTPISI_H
#define CONCURRENT_FILE_SEARCH_POTPISI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define BLOCK_SIZE 1024
#define MAX_FOLDERS 64
#define MAX_FILES 64
#define MAX_PATH_LENGTH 512
#define MAX_NAME_LENGTH 128
#define MAX_CHILDREN MAX_FOLDERS+MAX_FILES
#define BA_SIZE 64

#define WATCHER_SLEEP_TIME 5

typedef struct treeNode {
    char *fullPath;
    char *name; // just the name of file/dir
    struct treeNode *children[MAX_CHILDREN]; // arr of pointers to children, if node is dir
    struct treeNode *root; // all children will have a pointer to their isRoot
    // for cli commands
    int isFile; // 1 if file, 0 if dir
    int partial; // 1 if res is partial
    int currentRes; // current number of prime numbers in file
} treeNode;

typedef struct workerArgs {
    int id;
} workerArgs;

typedef struct watcherArgs {
    int *toTerminate;
    int isRoot; // 1 if watcher is root
    char *path; // full path
    pthread_t *self; // keep your own handle so you can free it
    treeNode *dirNode; // pointer to node in tree that the watcher is watching
} watcherArgs;

typedef struct fileInfo {
    char *name; // filename without full path before it
    char *lastModified; // returned by stat
} fileInfo;

typedef struct filesAndFolders {
    fileInfo **files; // array of all non-dir files in watched dir
    char **folders;
} filesAndFolders;

watcherArgs *rootWatcherArgsArray[64];
treeNode *treeRoots[64];
char cwd[MAX_PATH_LENGTH] = "";

// blocking array synching
sem_t empty;
sem_t full;
sem_t blockingArrayMutex;
int indexIn = 0;
int indexOut = 0;
treeNode *blockingArray[BA_SIZE];

int isNumberPrime(int input);

void helpMenu();

int makeWatcher(char *currPath, int *toTerminate, treeNode *parent, watcherArgs *initArgs);

int parseCommand(char *command);

treeNode *makeNewTreeNode(char parentPath[MAX_PATH_LENGTH], char currName[MAX_NAME_LENGTH], int isFile, treeNode *root);

void printTreeNode(treeNode *nodeToPrint, int reqRes);

void freeTreeNode(treeNode *t);

treeNode *findNodeWithPath(treeNode *root, char *path);

void removeChild(treeNode *childToRemove, treeNode *parent);

treeNode *addChildToParent(treeNode *parentNode, char *currName, int isFile);

// dir & file operations
void buildPath(char parentPath[MAX_PATH_LENGTH], char currentName[MAX_NAME_LENGTH],
               char fullFilePath[MAX_PATH_LENGTH]);

int scanDir(char ownPath[MAX_PATH_LENGTH], filesAndFolders *faf, treeNode *ownNode, int *toTerminate);

int scanFile(treeNode *nodeToScan, int workerID);

int folderIsNew(filesAndFolders *faf, char *currFolder);

int fileIsNewOrModified(filesAndFolders *faf, char *currFileName, char modfTime[50]);

void getLastModificationTime(char parentPath[MAX_PATH_LENGTH], char *currFileName, char returnTime[50]);

void searchFileForNumbers(FILE *f, int *currentRes, int workerID, int *searchCompleted, char *fileName);

// pthread functions
void *watcher(void *_args);

_Noreturn void *worker(void *_args);

// blocking array functions
treeNode *addToBA(treeNode *nodeToAdd);

treeNode *readFromBA();

#endif //CONCURRENT_FILE_SEARCH_POTPISI_H