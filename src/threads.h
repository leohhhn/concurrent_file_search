#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "helpers/declarations.h"
#include "helpers/threadHelpers.h"
#include "blockingArray.h"

int scanFile(treeNode *nodeToScan, int workerID) {
    FILE *f = fopen(nodeToScan->fullPath, "r");

    if (!f) {
        printf("Failed to open file at %s\n", nodeToScan->fullPath);
        return -1; // err handled in worker function
    }

    nodeToScan->partial = 1;
    searchFileForNumbers(f, &(nodeToScan->currentRes), workerID, &(nodeToScan->partial), nodeToScan->name);

    fclose(f);
    return 0;
}

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

            // todo do printf here for checking children's children

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
//                        printf("\nNew file found!\nName: %s\ncurrent modified time: %s\n\n",
//                               faf->files[i]->name,
//                               faf->files[i]->lastModified);
                        break;
                    }
                }

                treeNode *newFileNode = addChildToParent(ownNode, entry->d_name, 1); // todo free this mem
                addToBA(newFileNode);
                printf("Added node %s to BA\n", newFileNode->name);

            } else if (modifySwitch == 2) { // existing file modified
                printf("\n%s was modified!\nNew modified time: %s\n", entry->d_name, modifiedTime);
                printf("Adding %s to BA Again!\n\n", entry->d_name);

                char tmp[MAX_PATH_LENGTH] = "";
                buildPath(ownPath, entry->d_name, tmp);

                // give just parent instead of root
                treeNode *modifiedFileNode = findNodeWithPath(ownNode, tmp);

                // printTreeNode(modifiedFileNode, 0);
                if (modifiedFileNode) {
                    modifiedFileNode->currentRes = 0;
                    modifiedFileNode->partial = 0;
                    addToBA(modifiedFileNode);
                } else
                    printf("scanDir:: Err adding modified file to BA again.\n");
            }
        }
    }

    // remove deleted files from faf
    for (int i = 0; i < MAX_FILES; ++i) {
        if (!faf->files[i]) continue;
        char tmp[MAX_PATH_LENGTH] = "";
        buildPath(ownPath, faf->files[i]->name, tmp);
        FILE *f = fopen(tmp, "r");
        if (!f) {
            printf("\nFile %s was deleted from the FS and program.\n", faf->files[i]->name);
            removeChild(findNodeWithPath(ownNode, tmp), ownNode);
            free(faf->files[i]->name);
            free(faf->files[i]->lastModified);
            free(faf->files[i]);
            faf->files[i] = NULL;
            break;
        } else
            fclose(f);
    }

    return 0;
}




