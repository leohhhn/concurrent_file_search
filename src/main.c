#include "threads.h"
#include "helpers/generalHelpers.h"

sem_t empty;
sem_t full;
sem_t blockingArrayMutex;

int numOfWorkers;
int indexIn = 0;
int indexOut = 0;

pthread_t *workerThreads; // malloced array of pthread workers
watcherArgs rootWatcherArgsArray[64]; // global array for console to interact with watchers

void *watcher(void *_args) {
    watcherArgs *args = (watcherArgs *) _args; // points to an element in global rootWatcherArgs for a root watcher
    printf("New watcher created at %s\n", args->path + 54);

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
            printf("Dir at %s to be deleted from system. Watcher & its children will be exiting.\n", args->path + 54);
            break;
        }

        // scan dir & create children watchers
        int success = scanDir(args->path, faf, args->dirNode, args->toTerminate);

        if (success == -1) {
            // todo update result tree
            printf("Dir at %s deleted in the OS filesystem. Watcher & children will be exiting.\n", args->path + 54);
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
    printf("From thread: %d\n", ((workerArgs *) a)->id);

    while (1) {



        // get file


        sleep(2);
    }

}

_Noreturn void *cli() {
    numOfWorkers = 0;
    printf("Unesite broj Worker niti: ");
    scanf("%d%*c", &numOfWorkers);

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, numOfWorkers);

    workerThreads = malloc(numOfWorkers * sizeof(pthread_t));

    workerArgs wargs[numOfWorkers];

    for (int i = 0; i < numOfWorkers; i++) {
        wargs[i] = (workerArgs) {i, 1};
        printf("creating thread %d with args: id=%d avail=%d\n", i, wargs[i].id, wargs[i].available);
        pthread_create(workerThreads + i, NULL, worker, wargs + i);
    }

    int i = 0, cmd = 0;
    do {
        char *s = malloc(sizeof(char) * 128);
        for (int j = 0; j < strlen(s); j++)
            s[j] = '0';
        printf("Enter a command:\n");
        fgets(s, sizeof(s), stdin);
        cmd = parseCommand(s);
        free(s);

//        switch (cmd) {
//            case 1:
//
//                break;
//            case 2:
//                break;
//            case 3:
//                break;
//            case -1:
//                break;
//            default:
//                helpMenu();
//                break;
//        }
        sleep(10);

    } while (cmd != -1);

    sem_close(&full);
    sem_close(&empty);
    free(workerThreads);
    // free(blockingArray);
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

//    pthread_t commandLineThread;
//    pthread_create(&commandLineThread, NULL, cli, NULL);
//
//    pthread_join(commandLineThread, NULL);

    pthread_t watcherr;
    watcherArgs *r = malloc(sizeof(watcherArgs));
    r->path = malloc(sizeof(char) * MAX_PATH_LENGTH);
    strcpy(r->path, "/home/lav/Desktop/faks 2122/SRV/concurrent_file_search/");

    r->toTerminate = malloc(sizeof(int));
    r->dirNode = makeNewTreeNode(r->path, "", 0);
    *(r->toTerminate) = 0;
    r->root = 1;
    r->self = &watcherr;
    rootWatcherArgsArray[0] = *r;

    blockingArray = malloc(sizeof(char *) * BA_SIZE);
    for (int i = 0; i < BA_SIZE; i++) {
        blockingArray[i] = malloc(sizeof(char) * MAX_PATH_LENGTH);
    }

    pthread_create(&watcherr, NULL, watcher, &(rootWatcherArgsArray[0]));

    pthread_join(watcherr, NULL);
    return 0;
}