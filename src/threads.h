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

    printf("Opened file at %s\n", nodeToScan->fullPath);
    nodeToScan->searchCompleted = 0;
    searchFileForNumbers(f, &(nodeToScan->currentRes), workerID, &(nodeToScan->searchCompleted), nodeToScan->name);

    return 0;
}

int scanDir(char ownPath[MAX_PATH_LENGTH], filesAndFolders *faf, treeNode *ownNode, int *toTerminate) {
    // printf("Scanning dir at %s\n", name);
    DIR *folder = opendir(ownPath);
    struct dirent *entry;

    if (folder == NULL) return -1;

    while ((entry = readdir(folder))) {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            if (!folderIsNew(faf, entry->d_name)) continue;

            int watcherCreated = makeWatcher(entry->d_name, toTerminate, ownNode, NULL);
            if (!watcherCreated) continue;

            // copy folder name into folder array inside faf & into tree
            strcpy(faf->folders[faf->folderNum++], entry->d_name); // todo change to null & malloc

        } else if (entry->d_type == DT_REG) {
            char modifiedTime[50];
            getLastModificationTime(ownPath, entry->d_name, modifiedTime);

            int modifySwitch = fileIsNewOrModified(faf, entry->d_name, modifiedTime);

            if (modifySwitch == 0) {
                continue;
            } else if (modifySwitch == 1) { // totally new file
                strcpy(faf->files[faf->fileNum].name, entry->d_name);
                strcpy(faf->files[faf->fileNum++].lastModified, modifiedTime);
//                printf("\nNew file found!\nName: %s\ncurrent modified time: %s\n\n",
//                       faf->files[faf->fileNum - 1].name,
//                       faf->files[faf->fileNum - 1].lastModified);

                treeNode *newFileNode = addChildToParent(ownNode, entry->d_name, 1);
                addToBA(newFileNode);
                printf("Added node %s to BA\n", newFileNode->name);

            } else if (modifySwitch == 2) { // existing file modified
//                printf("\n%s was modified!\nNew modified time: %s\n\n",
//                       faf->files[faf->fileNum - 1].name, modifiedTime);
                //  addToBA();
            }
        }
    }
    return 0;
}



