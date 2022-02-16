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

treeNode *
makeNewTreeNode(char parentPath[MAX_PATH_LENGTH], char currName[MAX_NAME_LENGTH], int isFile, treeNode *root) {

    treeNode *t = malloc(sizeof(treeNode));
    t->isFile = isFile;
    t->fullPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    t->name = malloc(sizeof(char) * MAX_PATH_LENGTH);
    t->root = root;
    t->currentRes = 0;
    t->partial = 0;

    if (strcmp(currName, "") == 0) { // if creating root node
        strcpy(t->fullPath, parentPath);
        t->root = t; // if you're root, point to your self as root
        char *lastSlash = strrchr(t->fullPath, '/');
        if (lastSlash != NULL)
            strcpy(t->name, lastSlash + 1);
        else {
            printf("makeNewTreeNode:: could not find slash in path\n"); // todo add relative path
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
        if (!parentNode->children[i]) {
            parentNode->children[i] = makeNewTreeNode(parentNode->fullPath, currName, isFile, parentNode->root);
            return parentNode->children[i];
        }
    }
}


void printTreeNode(treeNode *nodeToPrint, int reqRes) {
    // if printing result for a single file
    if (reqRes) {
        if (nodeToPrint->partial)
            printf("Result for file %s: %d\n", nodeToPrint->fullPath, nodeToPrint->currentRes);
        else
            printf("Result for file %s: %d*\n", nodeToPrint->fullPath, nodeToPrint->currentRes);
        return;
    }

    printf("\nNode:\n");
    printf("fullPath: %s\n", nodeToPrint->fullPath);
    printf("name: %s\n", nodeToPrint->name);
    printf("isFile: %d\npartial: %d\n", nodeToPrint->isFile, nodeToPrint->partial);
    printf("currentRes: %d\n", nodeToPrint->currentRes);

    // todo find bug with root always being null
    if (nodeToPrint->root == NULL)
        printf("This is a root node!\n");
    else
        printf("Root: %s\n", nodeToPrint->root->name);
    printf("Children:\n");

    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (nodeToPrint->children[i])
            printf("Child %d - %s\n", i, nodeToPrint->children[i]->name);
    }
    printf("\n");
}

void removeChild(treeNode *childToRemove, treeNode *parent) {
    // removing is just setting the pointer to null
    if (!childToRemove) return; // failsafe

    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (childToRemove == parent->children[i])
            parent->children[i] = NULL;
    }
    freeTreeNode(childToRemove);
    childToRemove = NULL;
    return;
}

// returns pointer to node with given path, otherwise NULL
treeNode *findNodeWithPath(treeNode *ancestor, char *path) {
    printf("\nComparing:\n%s and\n%s\n\n", ancestor->fullPath, path);
    if (strcmp(ancestor->fullPath, path) == 0) {
        return ancestor;
    } else {
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (ancestor->children[i]) {
                return findNodeWithPath(ancestor->children[i], path);
            }
        }
    }
    return NULL;
}

// Frees malloced memory for treeNode
void freeTreeNode(treeNode *t) {
    free(t->fullPath);
    free(t->name);
    //  free(t->root);
    if (t->isFile) {
        free(t);
        return;
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
        freeTreeNode(t->children[i]);
        t->children[i] = NULL;
    }
    t = NULL;
    free(t);
    return;
}

// Prints tree given the root
void printTree(treeNode *root, int level) {
    if (!root) return;

    char tmp[32] = "";
    for (int i = 0; i < level; i++)
        strcat(tmp, "-");

    if (root->partial)
        printf("%s%s: %d*, partial: %d\n", tmp, root->name, root->currentRes, root->partial);
    else
        printf("%s%s: %d\n", tmp, root->name, root->currentRes);

    for (int i = 0; i < MAX_CHILDREN; ++i)
        if (root->children[i])
            printTree(root->children[i], level + 1);
}

typedef struct refreshRet {
    int res;
    int partial;
} refreshRet;

// Refreshes tree results
refreshRet refreshTreeResults(treeNode *root) {
    if (!root) return (refreshRet) {0, 0};

    refreshRet r = {0, 0};

    for (int i = 0; i < 64; i++) {
        if (root->children[i] && root->children[i]->isFile) {
            r.res += root->children[i]->currentRes;
            r.partial += root->children[i]->partial;
        } else if (root->children[i] && !root->children[i]->isFile) {
            refreshRet tmp = refreshTreeResults(root->children[i]);
            r.partial += tmp.partial;
            r.res += tmp.res;
        }
    }

    root->currentRes = r.res;
    root->partial = r.partial;
    return r;
}

void resetDirPartials(treeNode *root) {
    if (!root) return;

    if (!root->isFile) root->partial = 0;

    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (root->children[i] && !root->children[i]->isFile) {
            root->children[i]->partial = 0;
            resetDirPartials(root->children[i]);
        }
    }
}

void printCurrentRoots() {
    for (int i = 0; i < 64; ++i) {
        if (treeRoots[i])
            printTreeNode(treeRoots[i], 0);
    }
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
        printf("Please add an argument to your command. Type \"help\" to see usages.\n");
        return 0;
    }

    // if path is relative strcat it to cwd
    if (parsedCommand[1][0] != '/') {
        char tmp[MAX_NAME_LENGTH] = "";
        strcpy(tmp, parsedCommand[1]);
        strcpy(parsedCommand[1], cwd);
        strcat(parsedCommand[1], tmp);
    }

    printf("Parsed command is: %s %s\n", parsedCommand[0], parsedCommand[1]);

    if (strcmp(parsedCommand[0], "add_dir") == 0) {
        // check if dir is in system already

        if (!isDirectory(parsedCommand[1])) {
            printf("Given path must be a directory.\n");
            return 0;
        }

        for (i = 0; i < 64; i++) {
            if (treeRoots[i] && findNodeWithPath(treeRoots[i], parsedCommand[1])) {
                printf("Directory %s already in the system.\n", parsedCommand[1]);
                return 0;
            }
        }


        // create treeNode & rootWatcher
        int treeIndex = -1, wArgsIndex = -1; // find indexes
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

        FILE *f = fopen(parsedCommand[1], "r");
        if (f == NULL) {
            printf("Could not find dir at specified path. Try again.\n");
            return 0;
        };

        // make watcher args
        rootWatcherArgsArray[wArgsIndex] = malloc(sizeof(watcherArgs));
        watcherArgs *tmpArgs = rootWatcherArgsArray[wArgsIndex];
        pthread_t *rootWatcher = malloc(sizeof(pthread_t));

        tmpArgs->path = malloc(sizeof(char) * MAX_PATH_LENGTH);
        strcpy(tmpArgs->path, parsedCommand[1]);
        tmpArgs->toTerminate = malloc(sizeof(int));
        tmpArgs->dirNode = makeNewTreeNode(parsedCommand[1], "", 0, NULL);
        *(tmpArgs->toTerminate) = 0;
        tmpArgs->root = 1;
        tmpArgs->self = rootWatcher;

        // set root treeNode
        treeRoots[treeIndex] = rootWatcherArgsArray[wArgsIndex]->dirNode;

        //  make watcher
        makeWatcher(NULL, NULL, NULL, tmpArgs);


        return 0;
    } else if (strcmp(parsedCommand[0], "result") == 0) {
        // refresh all trees
        for (i = 0; i < 64; i++) {
            if (treeRoots[i]) {
                resetDirPartials(treeRoots[i]);
                refreshTreeResults(treeRoots[i]);
            }
        }

        // prints all trees if no arg
        if (strcmp(parsedCommand[1], "") == 0) {
            for (i = 0; i < 64; i++) {
                if (treeRoots[i]) {
                    printTree(treeRoots[i], 0);
                    return 0;
                }
            }
        }

        // prints single tree given root arg
        for (i = 0; i < 64; i++) {
            if (treeRoots[i] && strcmp(treeRoots[i]->fullPath, parsedCommand[1]) == 0) {
                printTree(treeRoots[i], 0);
                return 0;
            }
        }

        // find single node, dir or file
        treeNode *nodeToPrint = NULL;
        for (i = 0; i < 64; ++i) {
            if (treeRoots[i]) {
                printf("Looking into root: %s\n", treeRoots[i]->name);
                nodeToPrint = findNodeWithPath(treeRoots[i], parsedCommand[1]);
                if (nodeToPrint)
                    break;
            }
        }

        if (!nodeToPrint) {
            printf("could not find node with path %s\n", parsedCommand[1]);
            return 0;
        }

        printTreeNode(nodeToPrint, nodeToPrint->isFile);
        return 0;

    } else if (strcmp(parsedCommand[0], "help") == 0) {
        helpMenu();
        return 0;
    } else if (strcmp(parsedCommand[0], "remove_dir") == 0) {
        // todo add remove_dir funcitonality

        for (i = 0; i < 64; ++i) {
            if (treeRoots[i] && strcmp(treeRoots[i]->fullPath, parsedCommand[1]) == 0) {

            }
        }


        return 0;
    } else if (strcmp(parsedCommand[0], "q") == 0) {
        // quits program
        return -1;
    } else if (strcmp(parsedCommand[0], "printChild") == 0) {
        // prints child of given node, if possible
        treeNode *nodeToPrint = NULL;
        for (i = 0; i < 64; ++i) {
            if (treeRoots[i]) {
                nodeToPrint = findNodeWithPath(treeRoots[i], parsedCommand[1]);
                if (nodeToPrint)
                    break;
            }
        }

        printTreeNode(nodeToPrint, nodeToPrint->isFile);
        return 0;
    } else {
        printf("Invalid command. Please try again, or type \"help\" to see a list of valid commands.\n");
        return 0;
    }

    return -1;
}






