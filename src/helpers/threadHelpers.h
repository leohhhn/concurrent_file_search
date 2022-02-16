// initArgs - not null only if creating isRoot watcher
int makeWatcher(char *currPath, int *toTerminate, treeNode *parent, watcherArgs *initArgs) {

    if (initArgs) {
        return !pthread_create(initArgs->self, NULL, watcher, (void *) initArgs);
    } else {
        pthread_t *t = malloc(sizeof(pthread_t));
        watcherArgs *wa = malloc(sizeof(watcherArgs));

        wa->path = malloc(sizeof(char) * MAX_PATH_LENGTH);
        wa->toTerminate = toTerminate;
        wa->self = t;
        wa->isRoot = 0;
        wa->dirNode = addChildToParent(parent, currPath, 0);
        buildPath(parent->fullPath, currPath, wa->path);

        return !pthread_create(t, NULL, watcher, (void *) wa);
    }
}
