
/// Creates a new watcher
/// @param currPath - dir path at which to create a watcher
/// @param toTerminate - shared toTerminate pointer for new watcher
/// @param parent - treeNode pointer to parent
/// @param initArgs - option to forward watcher args instead of building them here
/// @note - set initArgs to NULL when not forwarding args
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
