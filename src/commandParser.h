#include "declarations.h"
#include "helpers/treeHelpers.h"
#include "helpers/generalHelpers.h"


/// CLI command parser
/// @param command - command from stdin to be parsed
int parseCommand(char *command) {

    char parsedCommand[2][MAX_PATH_LENGTH]; // command and arg
    parsedCommand[0][0] = 0; // reset memory from last call
    parsedCommand[1][0] = 0;

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
    }

    if (strcmp(parsedCommand[0], "result") != 0 && strcmp(parsedCommand[0], "clear") != 0 &&
        strcmp(parsedCommand[0], "q") != 0 &&
        strcmp(parsedCommand[1], "") == 0) {
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

    if (strcmp(parsedCommand[0], "add_dir") == 0) {
        if (!isDirectory(parsedCommand[1])) {
            printf("Given path must be a directory.\n");
            return 0;
        }

        // check if dir is in system already
        for (i = 0; i < 64; i++) {
            if (treeRoots[i] && findNodeWithPath(treeRoots[i], parsedCommand[1])) {
                printf("Directory %s already in the system.\n", parsedCommand[1]);
                return 0;
            }
        }

        // find indexes
        int treeIndex = -1, wArgsIndex = -1;
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

        // make root watcher and his root treeNode
        rootWatcherArgsArray[wArgsIndex] = malloc(sizeof(watcherArgs));
        watcherArgs *tmpArgs = rootWatcherArgsArray[wArgsIndex];
        pthread_t *rootWatcher = malloc(sizeof(pthread_t));

        tmpArgs->path = malloc(sizeof(char) * MAX_PATH_LENGTH);
        strcpy(tmpArgs->path, parsedCommand[1]);
        tmpArgs->toTerminate = malloc(sizeof(int *));
        tmpArgs->dirNode = makeNewTreeNode(parsedCommand[1], "", 0, NULL);
        *(tmpArgs->toTerminate) = 0;
        tmpArgs->isRoot = 1;
        tmpArgs->self = rootWatcher;

        // set root treeNode in global array
        treeRoots[treeIndex] = rootWatcherArgsArray[wArgsIndex]->dirNode;

        makeWatcher(NULL, NULL, NULL, tmpArgs);

        return 0;
    } else if (strcmp(parsedCommand[0], "result") == 0) {
        // refresh all trees before printing anything
        for (i = 0; i < 64; i++) {
            if (treeRoots[i]) {
                resetDirPartials(treeRoots[i]);
                refreshTreeResults(treeRoots[i]);
            }
        }

        // print all trees if no arg
        if (allRes) {
            int noRoots = 1;
            for (i = 0; i < 64; i++)
                if (treeRoots[i]) {
                    printTree(treeRoots[i], 0);
                    noRoots = 0;
                }
            if (noRoots)
                printf("No root dirs currently in system.\n");
            return 0;
        }

        // print single tree given root arg
        for (i = 0; i < 64; i++) {
            if (treeRoots[i] && strcmp(treeRoots[i]->fullPath, parsedCommand[1]) == 0) {
                printTree(treeRoots[i], 0);
                return 0;
            }
        }

        // find node, dir or file
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
            printTreeNode(nodeToPrint, nodeToPrint->isFile); // if node is file, print file
        else
            printTree(nodeToPrint, 0); // otherwise, print everything below the node
        return 0;

    } else if (strcmp(parsedCommand[0], "help") == 0) {
        // print help menu
        helpMenu();
        return 0;
    } else if (strcmp(parsedCommand[0], "remove_dir") == 0) {
        // remove dir from program, set toTerminate signal for all watchers in that tree
        for (i = 0; i < 64; ++i) {
            if (rootWatcherArgsArray[i] && strcmp(rootWatcherArgsArray[i]->path, parsedCommand[1]) == 0) {
                *(rootWatcherArgsArray[i]->toTerminate) = 1;
                return 0;
            }
        }

        printf("Arg has to point to a root dir.\n");
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
}
