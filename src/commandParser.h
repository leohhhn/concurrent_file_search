#include "declarations.h"
#include "helpers/treeHelpers.h"
#include "helpers/generalHelpers.h"

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
    int allRes = 0;
    if (strcmp(parsedCommand[0], "result") == 0 && strcmp(parsedCommand[1], "") == 0) {
        allRes = 1; // if command is only "result"
        // todo fix command staying in stdin
    }

    if (strcmp(parsedCommand[0], "result") != 0 && strcmp(parsedCommand[1], "") == 0) {
        printf("Please enter an argument to your command. See \"help\" for more info.\n");
        return 0;
    }

    if (!allRes) {
        // if path is relative strcat it to cwd
        if (strcmp(parsedCommand[1], "") != 0 && parsedCommand[1][0] != '/') {
            char tmp[MAX_NAME_LENGTH] = "";
            strcpy(tmp, parsedCommand[1]);
            strcpy(parsedCommand[1], cwd);
            strcat(parsedCommand[1], tmp);
        }
    }

    printf("Parsed command is: %s %s\n\n", parsedCommand[0], parsedCommand[1]);

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
        tmpArgs->isRoot = 1;
        tmpArgs->self = rootWatcher;

        // set isRoot treeNode
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
        if (allRes) {
            for (i = 0; i < 64; i++) {
                if (treeRoots[i]) {
                    printTree(treeRoots[i], 0);
                    return 0;
                }
            }
        }

        // prints single tree given isRoot arg
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
                nodeToPrint = findNodeWithPath(treeRoots[i], parsedCommand[1]);
                if (nodeToPrint)
                    break;
            }
        }

        if (!nodeToPrint) {
            printf("\n%s does not exist in the system.\n", parsedCommand[1]);
            return 0;
        }

        if (nodeToPrint->isFile)
            printTreeNode(nodeToPrint, nodeToPrint->isFile);
        else
            printTree(nodeToPrint, 0);
        return 0;

    } else if (strcmp(parsedCommand[0], "help") == 0) {
        helpMenu();
        return 0;
    } else if (strcmp(parsedCommand[0], "remove_dir") == 0) {
        for (i = 0; i < 64; ++i) {
            if (rootWatcherArgsArray[i] && strcmp(rootWatcherArgsArray[i]->path, parsedCommand[1]) == 0) {
                *(rootWatcherArgsArray[i]->toTerminate) = 1;
                return 0;
            }
        }
        // todo fix remove_dir not working with abs path
        printf("Could not find dir in program. Arg has to point to a root dir.\n");
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
