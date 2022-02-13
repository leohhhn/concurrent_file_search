#include "threads.h"
#include "helpers/generalHelpers.h"

int numOfWorkers;
pthread_t *workerThreads; // malloced array of pthread workers

void *watcher(void *_args) {
    watcherArgs *args = (watcherArgs *) _args; // points to an element in global rootWatcherArgs for a root watcher
    //printf("watcher:: New watcher created at %s\n", args->path);

    // allocating memory
    filesAndFolders *faf = malloc(sizeof(filesAndFolders));
    faf->folders = malloc(sizeof(char *) * MAX_FOLDERS);
    faf->files = malloc(sizeof(fileInfo) * MAX_FILES);
    faf->fileNum = 0;
    faf->folderNum = 0;

    for (int i = 0; i < MAX_FOLDERS; i++) {
        faf->folders[i] = malloc(sizeof(char) * MAX_NAME_LENGTH); // mallocing the subarray of chars
        faf->files[i].name = malloc(sizeof(char) * MAX_NAME_LENGTH);
        faf->files[i].lastModified = malloc(sizeof(char) * 50);
    }

    // watching
    while (1) {
        if (*(args->toTerminate) == 1) {
            // toTerminate will be set directly from the console, as root watcher args are in a global array
            printf("Dir at %s to be deleted from system. Watcher & its children will be exiting.\n", args->path);
            break;
        }

        // scan dir & create children watchers
        int success = scanDir(args->path, faf, args->dirNode, args->toTerminate);

        if (success == -1) {
            // todo update result tree
            printf("Dir at %s can't be found or was deleted in the OS filesystem. Watcher & children will be exiting.\n",
                   args->path);
            break;
        }

        sleep(WATCHER_SLEEP_TIME);
    }

    // freeing memory
    for (int i = 0; i < MAX_FILES; i++) {
        free(faf->folders[i]);
        free(faf->files[i].name);
        free(faf->files[i].lastModified);
    }
    free(faf->folders);
    free(faf->files);
    free(faf);
    if (args->root != 1) {
        free(args->self);
        free(args->path);
        free(args);
    }
    pthread_exit(NULL);
}

_Noreturn void *worker(void *a) {
    workerArgs *args = (workerArgs *) a;

    while (1) {
        treeNode *nodeToScan = readFromBA();
        int scanning = scanFile(nodeToScan, args->id);
    }
}

_Noreturn void *cli() {
    numOfWorkers = 0;
    printf("Unesite broj Worker niti: ");

    char tmp[5];
    fgets(tmp, sizeof(tmp), stdin);
    numOfWorkers = atoi(tmp);

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, numOfWorkers);
    sem_init(&blockingArrayMutex, 0, 1);

    workerThreads = malloc(numOfWorkers * sizeof(pthread_t));
    workerArgs wargs[numOfWorkers];

    for (int i = 0; i < numOfWorkers; i++) {
        wargs[i] = (workerArgs) {i, 1};
        //  printf("creating thread %d with args: id=%d avail=%d\n", i, wargs[i].id, wargs[i].available);
        pthread_create(workerThreads + i, NULL, worker, wargs + i);
    }

    int cmd = 0;

    do {
        char *s = malloc(sizeof(char) * (MAX_PATH_LENGTH + 128));

        printf("Enter a command:\n");
        fgets(s, MAX_PATH_LENGTH + 128, stdin);
        cmd = parseCommand(s);

        free(s);
    } while (cmd != -1);

    printf("\nexiting cli thread\n");

    sem_close(&full);
    sem_close(&empty);
    free(workerThreads);
    pthread_exit(NULL);
}

void helpMenu() {
    printf("Here are the commands you can use:\n");
    printf("add_dir <abs_or_relative_path> - add a root directory to the program, without a / in the end\n");
    printf("remove_dir <abs_or_relative_path> - remove a root directory & all its children from the program\n");
    printf("q - quit the program\n");
    printf("result <abs_or_relative_path> - print result for directory or specific file\n");
}

int main(int argc, char *argv[]) {

    char cwd[MAX_PATH_LENGTH] = "";
    strcpy(cwd, argv[0]);
    cwd[strlen(cwd) - 4] = '\0';

    for (int i = 0; i < 64; i++) {
        rootWatcherArgsArray[i] = NULL;
        treeRoots[i] = NULL;
        blockingArray[i] = NULL;
    }

    pthread_t commandLineThread;
    pthread_create(&commandLineThread, NULL, cli, NULL);

    pthread_join(commandLineThread, NULL);

    return 0;
}