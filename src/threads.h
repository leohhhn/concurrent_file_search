#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "declarations.h"
#include "helpers/threadHelpers.h"
#include "blockingArray.h"

/// Opens file and scans for prime numbers
/// @param nodeToScan - pointer to treeNode of file to scan
int scanFile(treeNode *nodeToScan) {
    FILE *f = fopen(nodeToScan->fullPath, "r");

    if (!f) {
        printf("Failed to open file at %s\n", nodeToScan->fullPath);
        return -1; // err handled in worker function
    }

    nodeToScan->partial = 1;
    searchFileForPrimeNumbers(f, &(nodeToScan->currentRes), &(nodeToScan->partial));

    fclose(f);
    return 0;
}

/// Scans a given dir and handles watcher's memory
/// @param faf - watcher's faf
/// @param ownNode - node being watched by watcher
/// @param toTerminate - signal for termination of all watchers in a tree. all watchers in a tree share the same pointer
int scanDir(char ownPath[MAX_PATH_LENGTH], filesAndFolders *faf, treeNode *ownNode, int *toTerminate) {
    DIR *folder = opendir(ownPath);
    struct dirent *entry;

    if (!folder) return -1;

    while ((entry = readdir(folder))) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            if (!folderIsNew(faf, entry->d_name)) continue;

            int watcherCreated = makeWatcher(entry->d_name, toTerminate, ownNode, NULL);
            if (!watcherCreated) continue;

            // copy folder name into folder array inside faf & into tree
            for (int i = 0; i < MAX_FOLDERS; ++i) {
                if (!faf->folders[i]) {
                    faf->folders[i] = malloc(sizeof(char) * MAX_NAME_LENGTH);
                    strcpy(faf->folders[i], entry->d_name);
                    break;
                }
            }
            printf("Found a dir: %s! Making a new watcher for it.\n", entry->d_name);

        } else if (entry->d_type == DT_REG) {
            char modifiedTime[50];
            getLastModificationTime(ownPath, entry->d_name, modifiedTime);

            int modifySwitch = fileIsNewOrModified(faf, entry->d_name, modifiedTime);

            if (modifySwitch == 0) { // not new & not modified
                continue;
            } else if (modifySwitch == 1) { // totally new file

                // malloc new fileInfo and write to it
                for (int i = 0; i < MAX_FILES; ++i) {
                    if (!faf->files[i]) {
                        faf->files[i] = malloc(sizeof(fileInfo *));
                        faf->files[i]->name = malloc(sizeof(char) * MAX_NAME_LENGTH);
                        faf->files[i]->lastModified = malloc(sizeof(char) * 50);

                        strcpy(faf->files[i]->name, entry->d_name);
                        strcpy(faf->files[i]->lastModified, modifiedTime);
                        break;
                    }
                }

                treeNode *newFileNode = addChildToParent(ownNode, entry->d_name, 1);
                addToBA(newFileNode);
                printf("Found a file: %s! Adding it to BA.\n", newFileNode->name);
            } else if (modifySwitch == 2) { // existing file modified
                printf("\n%s was modified! New modified time: %s\n", entry->d_name, modifiedTime);
                printf("Adding %s to BA again!\n\n", entry->d_name);

                char tmp[MAX_PATH_LENGTH] = "";
                buildPath(ownPath, entry->d_name, tmp);

                treeNode *modifiedFileNode = findNodeWithPath(ownNode, tmp);
                if (modifiedFileNode) {
                    modifiedFileNode->currentRes = 0;
                    modifiedFileNode->partial = 0;
                    addToBA(modifiedFileNode);
                } else
                    printf("scanDir:: Err adding modified file to BA again.\n");
            }
        }
    }

    // remove deleted files from faf & tree
    for (int i = 0; i < MAX_FILES; ++i) {
        if (!faf->files[i]) continue;
        char tmp[MAX_PATH_LENGTH] = "";
        buildPath(ownPath, faf->files[i]->name, tmp);
        FILE *f = fopen(tmp, "r");
        if (!f) {
            printf("\nFile %s was deleted from the FS and program.\n", faf->files[i]->name);
            treeNode *t = findNodeWithPath(ownNode, tmp);
            removeChild(t, ownNode);
            free(faf->files[i]->name);
            free(faf->files[i]->lastModified);
            free(faf->files[i]);
            faf->files[i] = NULL;
            break;
        } else
            fclose(f);
    }
    // remove deleted folders from faf & tree
    for (int i = 0; i < MAX_FOLDERS; ++i) {
        if (!faf->folders[i]) continue;
        char tmp[MAX_PATH_LENGTH] = "";
        buildPath(ownPath, faf->folders[i], tmp);
        DIR *dir = opendir(tmp);
        if (!dir) {
            printf("\nFolder %s was deleted from the FS and program.\n", faf->folders[i]);
            treeNode *t = findNodeWithPath(ownNode, tmp);
            removeChild(t, ownNode);
            free(faf->folders[i]);
            faf->folders[i] = NULL;
            break;
        } else
            closedir(dir);
    }

    return 0;
}




