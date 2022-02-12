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

void searchFileForNumbers(FILE *f, int *currentRes, int *workerID, int *done) {

//        // how big is the file?
//        fseek(f, 0, SEEK_END); // seek to end of file
//        long fileSize = ftell(f); // get current file pointer
//        fseek(f, 0, SEEK_SET); // seek back to beginning of file
//        int fileSizeInBlocks = ((int) fileSize) / BLOCK_SIZE;
//        int leftover = ((int) fileSize) % BLOCK_SIZE;

    int numberOfPrimes = 0, searchedSoFar = 0, i = 0;
    char *buffer = malloc(BLOCK_SIZE * sizeof(char));

    while (fgets(buffer, BLOCK_SIZE * sizeof(char), f) != 0) {
        searchedSoFar += (int) strlen(buffer);
        if (searchedSoFar / BLOCK_SIZE > i++) {
            // todo update res tree every BLOCK_SIZE
            *currentRes = numberOfPrimes;

            printf("Worker %d searched %d bytes, and found %d prime numbers so far.\n", *workerID,
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

    *done = 1;
    *currentRes = numberOfPrimes;

//     free(buffer); // todo see why error occurs here

}

void buildPath(char parentPath[MAX_PATH_LENGTH],
               char currentName[MAX_NAME_LENGTH],
               char fullFilePath[MAX_PATH_LENGTH]) {
    strcpy(fullFilePath, parentPath);
    strcat(fullFilePath, "/");
    strcat(fullFilePath, currentName);
}

// helper function, only called by addChildToParent
treeNode *makeNewTreeNode(char parentPath[MAX_PATH_LENGTH], char currName[MAX_NAME_LENGTH], int isFile) {
    treeNode *t = malloc(sizeof(treeNode));
    t->isFile = isFile;
    t->fullPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    t->name = malloc(sizeof(char) * MAX_PATH_LENGTH);

    buildPath(parentPath, currName, t->fullPath);
    strcpy(t->name, currName);

    printf("new node created at %s\n", t->fullPath);

    if (isFile) return t; // don't malloc children if leaf node

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
}

void printTree(treeNode *root, int level) {
    if (!root)
        return;
    char tmp[32] = "";

    for (int i = 0; i < level; i++) {
        strcat(tmp, "-");
    }

    if (level == 0)
        printf("%s\n", root->name);
    else
        printf("%s%s\n", tmp, root->name);

    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (root->children[i])
            printTree(root->children[i], level + 1);
    }
}

int parseCommand(char *command) {
    printf("%s", command);
    char fin[2][MAX_PATH_LENGTH]; // command and arg
    char *token = strtok(command, " ");
    strcpy(fin[0], token);
    int i = 0;

    while (token != NULL) {
        strcpy(fin[i++], token);
        token = strtok(NULL, " ");
    }

    printf("inputted command is: %s %s\n", fin[0], fin[1]);

}







