#include "../declarations.h"

treeNode *
makeNewTreeNode(char parentPath[MAX_PATH_LENGTH], char currName[MAX_NAME_LENGTH], int isFile, treeNode *root) {

    treeNode *t = malloc(sizeof(treeNode));
    t->isFile = isFile;
    t->fullPath = malloc(sizeof(char) * MAX_PATH_LENGTH);
    t->name = malloc(sizeof(char) * MAX_PATH_LENGTH);
    t->root = root;
    t->currentRes = 0;
    t->partial = 0;

    if (strcmp(currName, "") == 0) { // if creating isRoot node
        strcpy(t->fullPath, parentPath);
        t->root = t; // if you're isRoot, point to your self as isRoot
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
        if (!parentNode->children[i]) {
            parentNode->children[i] = makeNewTreeNode(parentNode->fullPath, currName, isFile, parentNode->root);
            return parentNode->children[i];
        }
    }
    return NULL;
}

void printTreeNode(treeNode *nodeToPrint, int reqRes) {
    // if printing result for a single file
    if (!nodeToPrint) {
        printf("\nNode to print is NULL\n");
        return;
    }

    if (reqRes) {
        if (nodeToPrint->partial)
            printf("\nNumber of primes in %s is *%d.\n\n", nodeToPrint->name, nodeToPrint->currentRes);
        else
            printf("\nNumber of primes in %s is %d.\n\n", nodeToPrint->name, nodeToPrint->currentRes);
        return;
    }

    printf("\nNode:\n");
    printf("fullPath: %s\n", nodeToPrint->fullPath);
    printf("name: %s\n", nodeToPrint->name);
    printf("isFile: %d\npartial: %d\n", nodeToPrint->isFile, nodeToPrint->partial);
    printf("currentRes: %d\n", nodeToPrint->currentRes);

    if (nodeToPrint->root == NULL)
        printf("This is a root node!\n");
    else
        printf("Root: %s\n", nodeToPrint->root->name);
    printf("Children:\n");

    for (int i = 0; i < MAX_CHILDREN; ++i) {
        if (nodeToPrint->children[i])
            printf("\tChild %d - %s\n", i, nodeToPrint->children[i]->name);
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
    if (strcmp(ancestor->fullPath, path) == 0) {
        return ancestor;
    } else {
        for (int i = 0; i < MAX_CHILDREN; i++) {
            if (ancestor->children[i]) {
                treeNode *res = findNodeWithPath(ancestor->children[i], path);
                if (res)
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
    //  free(t->isRoot);
    if (t->isFile) {
        free(t);
        return;
    }
    for (int i = 0; i < MAX_CHILDREN; i++) {
        if (t->children[i]) {
            freeTreeNode(t->children[i]);
            t->children[i] = NULL;
        }
    }
    t = NULL;
    free(t);
    return;
}

// Prints tree given the isRoot
void printTree(treeNode *root, int level) {
    if (!root) return;

    char tmp[32] = "";
    for (int i = 0; i < level; i++)
        strcat(tmp, "\t");

    if (root->partial)
        printf("%s%s: *%d, partial: %d\n", tmp, root->name, root->currentRes, root->partial);
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
