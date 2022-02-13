#include <math.h>
#include <string.h>

/// Returns 1 if number is prime, 0 otherwise
int isNumberPrime(int input) {
    if (input <= 1) return 0;
    for (int i = 2; i <= sqrt(input); i++)
        if (input % i == 0) return 0;
    // printf("%d\n", input);
    return 1;
}

void searchFileForNumbers(FILE *f, int *currentRes, int workerID, int *searchCompleted) {

//        // how big is the file?
//        fseek(f, 0, SEEK_END); // seek to end of file
//        long fileSize = ftell(f); // get current file pointer
//        fseek(f, 0, SEEK_SET); // seek back to beginning of file
//        int fileSizeInBlocks = ((int) fileSize) / BLOCK_SIZE;
//        int leftover = ((int) fileSize) % BLOCK_SIZE;

    int numberOfPrimes = 0, searchedSoFar = 0, i = 0;
    char *buffer = malloc(BLOCK_SIZE * sizeof(char));
//    char *tmp = buffer;

    while (fgets(buffer, BLOCK_SIZE * sizeof(char), f) != 0) {
        searchedSoFar += (int) strlen(buffer);
        if (searchedSoFar / BLOCK_SIZE > i++) {
            *currentRes = numberOfPrimes;

            printf("Worker %d searched %d bytes, and found %d prime numbers so far.\n", workerID,
                   searchedSoFar, *currentRes);
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

    *searchCompleted = 1;
    *currentRes = numberOfPrimes;

//   free(tmp);
}

void buildPath(char parentPath[MAX_PATH_LENGTH],
               char currentName[MAX_NAME_LENGTH],
               char fullFilePath[MAX_PATH_LENGTH]) {
    strcpy(fullFilePath, parentPath);
    strcat(fullFilePath, "/");
    strcat(fullFilePath, currentName);
    return;
}

treeNode *makeNewTreeNode(char parentPath[MAX_PATH_LENGTH], char currName[MAX_NAME_LENGTH], int isFile) {
    treeNode *t = malloc(sizeof(treeNode));
    t->isFile = isFile;
    t->fullPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    t->name = malloc(sizeof(char) * MAX_PATH_LENGTH);

    if (strcmp(currName, "") == 0) { // if creating root node
        strcpy(t->fullPath, parentPath);
        char *lastSlash = strrchr(t->fullPath, '/');
        if (lastSlash != NULL)
            strcpy(t->name, lastSlash + 1);
        else {
            printf("makeNewTreeNode:: could not find slash in path\n");
            return NULL;
        }
    } else {
        buildPath(parentPath, currName, t->fullPath);
        strcpy(t->name, currName);
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        t->children[i] = NULL; // will be malloced on a case-to-case basis
    }
    return t;
}

// Adds child to tree given child name and parent, returns pointer to added child
treeNode *addChildToParent(treeNode *parentNode, char *currName, int isFile) {
    // find first null spot to add to array
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (parentNode->children[i] == NULL) {
            parentNode->children[i] = makeNewTreeNode(parentNode->fullPath, currName, isFile);
            return parentNode->children[i];
        }
    }
}

void printTreeNode(treeNode *nodeToPrint) {
    printf("\nNode:\n");
    printf("fullPath: %s\n", nodeToPrint->fullPath);
    printf("name: %s\n", nodeToPrint->name);
    printf("isFile: %d\nsearchCompleted:%d\n", nodeToPrint->isFile, nodeToPrint->searchCompleted);
    printf("currentRes: %d\n", nodeToPrint->currentRes);
    printf("Children:\n");
    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (nodeToPrint->children[i])
            printf("Child %d - %s", i, nodeToPrint->children[i]->name);
    }
    printf("\n");
}

void removeChild(treeNode *childToRemove) {
    // removing is just setting the pointer to null
    freeTreeNode(childToRemove);
    childToRemove = NULL;
    return;
}

// returns pointer to node with given path, otherwise NULL
treeNode *findInTree(treeNode *root, char *path) {
    if (strcmp(root->fullPath, path) == 0) {
        return root;
    } else {
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (root->children[i]) {
                treeNode *res = findInTree(root->children[i], path);
                return res;
            }
        }
    }
    return NULL;
}

// Frees malloced memory for treeNode
void freeTreeNode(treeNode *t) {
    free(t->fullPath);
    free(t->name);
    if (t->isFile) {
        free(t);
        return;
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
        free(t->children[i]);
    }
    free(t);
    return;
}

void printTree(treeNode *root, int level) {
    if (!root)
        return;

    char tmp[32] = "";

    for (int i = 0; i < level; i++)
        strcat(tmp, "-");

    if (root->searchCompleted != 1)
        printf("%s%s: %d*\n", tmp, root->name, root->currentRes);
    else
        printf("%s%s: %d\n", tmp, root->name, root->currentRes);

    for (int i = 0; i < MAX_CHILDREN; ++i)
        if (root->children[i])
            printTree(root->children[i], level + 1);
}

int parseCommand(char *command) {
    char parsedCommand[2][MAX_PATH_LENGTH]; // command and arg
    command[strlen(command) - 1] = '\0'; // remove trailing newline char

    int i = 0;
    char *token = strtok(command, " ");

    while (token != NULL) {
        strcpy(parsedCommand[i++], token);
        token = strtok(NULL, " ");
        if (i == 2) break; // limiting input to only 2 separate "words"
    }

    if (strcmp(parsedCommand[0], "result") != 0 && strcmp(parsedCommand[1], "") == 0) {
        // todo fix - breaks if input is random & without argument
        printf("Please add argument to your command. Type \"help\" to see usages.\n");
        return 0;
    }

    printf("Parsed command is: %s %s\n", parsedCommand[0], parsedCommand[1]);

    if (strcmp(parsedCommand[0], "add_dir") == 0) {
        // check if dir is in system already
        for (i = 0; i < 64; i++) {
            if (treeRoots[i] && findInTree(treeRoots[i], parsedCommand[1])) {
                printf("Directory %s already in the system.\n", parsedCommand[1]);
                return 0;
            }
        }

        // create treeNode & rootWatcher
        int treeIndex = -1, wArgsIndex = -1;
        // find indexes
        for (i = 0; i < 64; i++) {
            if (treeRoots[i] == NULL) {
                treeIndex = i;
                break;
            }
        }
        for (i = 0; i < 64; i++) {
            if (rootWatcherArgsArray[i] == NULL) {
                wArgsIndex = i;
                break;
            }
        }

        if (treeIndex == -1 || wArgsIndex == -1) {
            printf("Cannot add more dirs to system.\n");
            return 0;
        }

        // make watcher args
        rootWatcherArgsArray[wArgsIndex] = malloc(sizeof(watcherArgs));
        watcherArgs *tmpArgs = rootWatcherArgsArray[wArgsIndex];
        pthread_t *rootWatcher = malloc(sizeof(pthread_t));

        tmpArgs->path = malloc(sizeof(char) * MAX_PATH_LENGTH);
        strcpy(tmpArgs->path, parsedCommand[1]);
        tmpArgs->toTerminate = malloc(sizeof(int));
        tmpArgs->dirNode = makeNewTreeNode(parsedCommand[1], "", 0);
        *(tmpArgs->toTerminate) = 0;
        tmpArgs->root = 1;
        tmpArgs->self = rootWatcher;

        // set root treeNode
        treeRoots[treeIndex] = rootWatcherArgsArray[wArgsIndex]->dirNode;

        //  make watcher
        makeWatcher(NULL, NULL, NULL, tmpArgs);
        return 0;
    } else if (strcmp(parsedCommand[0], "result") == 0) {
        for (i = 0; i < 64; i++) {
            if (treeRoots[i] && strcmp(treeRoots[i]->fullPath, parsedCommand[1]) == 0) {
                printTree(treeRoots[i], 0);
                return 0;
            }
        }
    } else if (strcmp(parsedCommand[0], "help") == 0) {
        helpMenu();
        return 0;
    } else if (strcmp(parsedCommand[0], "remove_dir") == 0) {
        // todo add remove_dir funcitonality
        return 0;
    } else {
        printf("Invalid command. Please try again, or type \"help\" to see a list of valid commands.\n");
        return 0;
    }

    return -1;
}






