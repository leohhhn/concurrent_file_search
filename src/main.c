#include "threads.h"
#include "declarations.h"
#include "commandParser.h"

int numOfWorkers;
pthread_t *workerThreads; // malloced array of pthread workers

/// Watcher thread function
void *watcher(void *_args) {
    watcherArgs *args = (watcherArgs *) _args; // points to an element in global rootWatcherArgs for a isRoot watcher

    // allocating memory
    filesAndFolders *faf = malloc(sizeof(filesAndFolders));
    faf->folders = malloc(sizeof(char *) * MAX_FOLDERS);
    faf->files = malloc(sizeof(fileInfo *) * MAX_FILES);

    for (int i = 0; i < MAX_FOLDERS; i++) {
        faf->folders[i] = NULL;
        faf->files[i] = NULL;
    }

    // watching
    while (1) {
        if (*(args->toTerminate) == 1) {
            // toTerminate will be set directly from the console, as isRoot watcher args are in a global array
            printf("Dir at %s was deleted from the system.\n", args->path);
            break;
        }

        // scan dir & create children watchers
        int success = scanDir(args->path, faf, args->dirNode, args->toTerminate);

        if (success == -1) {
            printf("Dir at %s can't be found or was deleted in the OS filesystem.\n",
                   args->path);
            break;
        }
        sleep(WATCHER_SLEEP_TIME);
    }

    // freeing memory
    free(faf->folders);
    free(faf->files);
    free(faf);
    if (args->isRoot != 1) {
        free(args->self);
        free(args->path);
        free(args);
    } else {
        for (int i = 0; i < 64; ++i) {
            if (treeRoots[i] && strcmp(treeRoots[i]->fullPath, args->path) == 0) {
                freeTreeNode(treeRoots[i]);
                treeRoots[i] = NULL;
            }
        }
    }
    pthread_exit(NULL);
}

/// Worker thread function
_Noreturn void *worker(void *a) {
    while (1) {
        treeNode *nodeToScan = readFromBA();
        scanFile(nodeToScan);
    }
}

/// CLI thread function
_Noreturn void *cli() {
    numOfWorkers = 0;
    printf("Enter the number of Worker threads: ");

    char tmp[5];
    fgets(tmp, 5, stdin);
    numOfWorkers = atoi(tmp);

    if (numOfWorkers <= 0) {
        printf("Number or workers must be at least 1. Please try again.\n");
        pthread_exit(NULL);
    }

    // init semaphores for blocking array
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, numOfWorkers);
    sem_init(&blockingArrayMutex, 0, 1);

    workerThreads = malloc(numOfWorkers * sizeof(pthread_t));
    for (int i = 0; i < numOfWorkers; i++) {
        pthread_create(workerThreads + i, NULL, worker, NULL);
    }

    int cmd = 0;
    do {
        char *s = malloc(sizeof(char) * (MAX_PATH_LENGTH + 128));
        fgets(s, MAX_PATH_LENGTH + 128, stdin);
        cmd = parseCommand(s);
        free(s);
    } while (cmd != -1);

    printf("\nProgram will now exit.\n");

    sem_close(&full);
    sem_close(&empty);
    free(workerThreads);
    pthread_exit(NULL);
}

/// Prints help menu to CLI
void helpMenu() {
    printf("Here are the commands you can use:\n");
    printf("add_dir <abs_or_relative_path> - add a isRoot directory to the program, without a / in the end\n");
    printf("remove_dir <abs_or_relative_path> - remove a root directory & all its children from the program\n");
    printf("q - quit the program\n");
    printf("result <abs_or_relative_path> - print result for directory or specific file\n\n");
}

int main(int argc, char *argv[]) {
    // get cwd
    getcwd(cwd, MAX_PATH_LENGTH);
    strcat(cwd, "/");

    // set init values for global arrays
    for (int i = 0; i < 64; i++) {
        rootWatcherArgsArray[i] = NULL;
        treeRoots[i] = NULL;
        blockingArray[i] = NULL;
    }

    // create CLI thread
    pthread_t commandLineThread;
    pthread_create(&commandLineThread, NULL, cli, NULL);
    pthread_join(commandLineThread, NULL);

    return 0;
}