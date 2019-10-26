#include "headers/command.h"

void tree(FILE *fat12, struct BootSector *bootSector)
{
    printf("[DIR]/:\n");

    char *fullNames[224];
    int logicalClusters[224];
    int index = 0;

    struct DirEntry dirEntry;
    struct DirEntry *de_ptr = &dirEntry;
    int base =
        (bootSector->BS_NumberOfReservedSectors + bootSector->BS_NumberOfFATs * bootSector->BS_SectorsPerFAT) * bootSector->BS_BytesPerSector;

    struct FileName fileName;
    for (size_t i = 0; i < bootSector->BS_MaxNumOfRootDirectoryEntries; i++, base += 32)
    {
        // printf("%d ", i);
        mapDirEntry(fat12, de_ptr, base);

        /*
		 *	2. If the first byte of the Filename field is 0xE5, then the directory entry is free (i.e., currently
		 * unused), and hence there is no file or subdirectory associated with the directory entry.
		 *	3. If the first byte of the Filename field is 0x00, then this directory entry is free and all the
		 * remaining directory entries in this directory are also free
		 */
        if (dirEntry.DE_Filename[0] == 0x00)
        {
            break;
        }
        else if (dirEntry.DE_Filename[0] == 0xE5)
        {
            continue;
        }

        // now current entry is in use
        char *fileName = extractFileName(dirEntry.DE_Filename);

        // skip hidden file or directories
        if ((dirEntry.DE_Attributes & 0x02) != 0)
        {
            continue;
        }

        if ((dirEntry.DE_Attributes & 0x10) != 0)
        {
            // SubDirectory
            printf("[DIR]%s ", fileName);

            char *fullName = (char *)malloc(100);
            memset(fullName, 0, 100);
            fullName[0] = '/';
            strcpy(fullName + 1, fileName);
            // ls(fat12, fullName, base);
            fullNames[index] = fullName;
            logicalClusters[index++] = dirEntry.DE_FirstLogicalCluster;
        }
        else
        {
            // file
            printf("[FILE]%s ", fileName);
        }
    }
    printf("\n");

    for (size_t i = 0; i < index; i++)
    {
        char *l_fullNames[224];
        int l_logicalClusters[224];
        ls(fat12, bootSector, fullNames[i], logicalClusters[i], l_fullNames, l_logicalClusters, 0);
    }
}

void ls(FILE *fat12, struct BootSector *bootSector, char *dir, int logicalCluster, char *fullNames[], int *logicalClusters, int dirNum)
{
    int base = getPhysicalBase(logicalCluster, bootSector);

    if (dirNum == 0)
        printf("[DIR]%s/:\n", dir);
    struct DirEntry dirEntry;
    for (size_t i = 0; i < DIR_ENTRY_PER_SECTOR; i++, base += 32)
    {
        // printf("%d ", i);
        mapDirEntry(fat12, &dirEntry, base);

        if (dirEntry.DE_Filename[0] == 0x00)
        {
            break;
        }
        else if (dirEntry.DE_Filename[0] == 0xE5)
        {
            continue;
        }

        // now DirEntry is in use
        char *fileName = extractFileName(dirEntry.DE_Filename);

        // skip hidden file or directories
        if ((dirEntry.DE_Attributes & 0x02) != 0)
        {
            continue;
        }

        if ((dirEntry.DE_Attributes & 0x10) != 0)
        {
            // SubDirectory
            printf("[DIR]%s ", fileName);

            if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
            {
                char *fullName = (char *)malloc(100);
                memset(fullName, 0, 100);
                strcpy(fullName, dir);
                int len = strlen(dir);
                fullName[len++] = '/';
                strcpy(fullName + len, fileName);
                // ls(fat12, fullName, base);
                fullNames[dirNum] = fullName;
                logicalClusters[dirNum++] = dirEntry.DE_FirstLogicalCluster;
            }
        }
        else
        {
            // file
            printf("[FILE]%s ", fileName);
        }
    }

    // check if it's the last cluster
    // if not, continue to scan the next cluster
    /**
     * Value            Meaning
     * 0x00             Unused
     * 0xFF0-0xFF6      Reserved cluster
     * 0xFF7            Bad cluster
     * 0xFF8-0xFFF      Last cluster in a file
     * (anything else)  Number of the next cluster in the file
     */
    u16 nextLogicalClusterValue = getNextLogicalCluster(fat12, logicalCluster, bootSector);
    // printf("next cluster: %d\n", nextLogicalClusterValue);
    if (nextLogicalClusterValue >= 0x0FF8 && nextLogicalClusterValue <= 0x0FFF)
    {
        printf("\n");

        // now there are no clusters left, continue to scan the subdirectories one by one
        for (size_t i = 0; i < dirNum; i++)
        {
            // ls(fat12, fullNames[i], logicalClusters[i]);
            char *l_fullNames[224];
            int l_logicalClusters[224];
            ls(fat12, bootSector, fullNames[i], logicalClusters[i], l_fullNames, l_logicalClusters, 0);
        }
    }
    else if (nextLogicalClusterValue > 0x001 && nextLogicalClusterValue < 0xFF0)
    {
        // ok, current cluster is not the last
        ls(fat12, bootSector, dir, nextLogicalClusterValue, fullNames, logicalClusters, dirNum);
    }
}