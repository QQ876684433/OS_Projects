#include "headers/command.h"

void countDirAndFile(FILE *fat12, struct BootSector *bootSector, int logicalCluster, int *flags, int entryNum)
{
    int base = getPhysicalBase(logicalCluster, bootSector);

    struct DirEntry dirEntry;
    for (size_t i = 0; i < DIR_ENTRY_PER_SECTOR; i++, base += 32)
    {
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
            if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
            {
                flags[entryNum++] = ENTRY_DIRECTORY;
            }
        }
        else
        {
            // file
            flags[entryNum++] = ENTRY_FILE;
        }
    }

    // check if it's the last cluster
    u16 nextLogicalClusterValue = getNextLogicalCluster(fat12, logicalCluster, bootSector);
    if (nextLogicalClusterValue > 0x001 && nextLogicalClusterValue < 0xFF0)
    {
        // ok, current cluster is not the last
        countDirAndFile(fat12, bootSector, nextLogicalClusterValue, flags, entryNum);
    }
}

void lsPrint(FILE*fat12, struct BootSector* bootSector, int hasParam, char *dir, char *directoryEntries[], int dirLogicalClusters[], int *flags, int entryNum){
    if (hasParam == LS_NO_PARAM)
    {
        printf("%s/:\n", dir);
        for (size_t i = 0; i < entryNum; i++)
        {
            char *sp[224];
            int count = splits(directoryEntries[i], sp, '/');
            printf("%s  ", sp[count - 1]);
        }
    }
    else
    {
        int dn = 0, fn = 0;
        for (size_t i = 0; i < entryNum; i++)
        {
            if (flags[i] == ENTRY_DIRECTORY)
            {
                dn++;
            }
            else if (flags[i] == ENTRY_FILE)
            {
                fn++;
            }
        }

        printf("%s %d %d:\n", strcmp(dir, "")==0?"/":dir, dn, fn);
        for (size_t i = 0; i < entryNum; i++)
        {
            if (flags[i] == ENTRY_DIRECTORY)
            {
                int logClstr = dirLogicalClusters[i];
                int dirNum = 0, fileNum = 0;
                int flags[224];
                countDirAndFile(fat12, bootSector, logClstr, flags, 0);
                char *sp[224];
                int count = splits(directoryEntries[i], sp, '/');
                printf("%s %d %d\n", sp[count - 1], dirNum, fileNum);
            }
            else
            {
                printf("%s\n", directoryEntries[i]);
            }
        }
    }
    printf("\n");
}

/**
 * ======================================================================================================================================================
 * ======================================================================================================================================================
 */

void tree(FILE *fat12, struct BootSector *bootSector, int hasParam)
{
    char *directoryEntries[224];
    int logicalClusters[224];
    int flags[224];
    int entryNum = 0;

    struct DirEntry dirEntry;
    struct DirEntry *de_ptr = &dirEntry;
    int base =
        (bootSector->BS_NumberOfReservedSectors + bootSector->BS_NumberOfFATs * bootSector->BS_SectorsPerFAT) * bootSector->BS_BytesPerSector;

    struct FileName fileName;
    for (size_t i = 0; i < bootSector->BS_MaxNumOfRootDirectoryEntries; i++, base += 32)
    {
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
        if (isHidden(dirEntry.DE_Attributes))
        {
            continue;
        }

        if (isSubDirectory(dirEntry.DE_Attributes))
        {
            // SubDirectory
            char *fullName = (char *)malloc(100);
            memset(fullName, 0, 100);
            fullName[0] = '/';
            strcpy(fullName + 1, fileName);
            directoryEntries[entryNum] = fullName;
            logicalClusters[entryNum] = dirEntry.DE_FirstLogicalCluster;
            flags[entryNum] = ENTRY_DIRECTORY;
        }
        else
        {
            // file
            directoryEntries[entryNum] = fileName;
            logicalClusters[entryNum] = dirEntry.DE_FirstLogicalCluster;
            flags[entryNum] = ENTRY_FILE;
        }
        entryNum++;
    }

    lsPrint(fat12, bootSector, hasParam, "", directoryEntries, logicalClusters, flags, entryNum);

    for (size_t i = 0; i < entryNum; i++)
    {
        if (flags[i] == ENTRY_DIRECTORY)
        {
            char *l_directoryEntries[224];
            int l_dirLogicalClusters[224];
            int l_flags[224];
            ls(fat12, bootSector, hasParam, directoryEntries[i], logicalClusters[i], l_directoryEntries, l_dirLogicalClusters, l_flags, 0);
        }
    }
}

void ls(FILE *fat12, struct BootSector *bootSector, int hasParam, char *dir, int logicalCluster, char *directoryEntries[], int *dirLogicalClusters, int *flags, int entryNum)
{
    int base = getPhysicalBase(logicalCluster, bootSector);

    struct DirEntry dirEntry;
    for (size_t i = 0; i < DIR_ENTRY_PER_SECTOR; i++, base += 32)
    {
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
            if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
            {
                char *fullName = (char *)malloc(100);
                memset(fullName, 0, 100);
                strcpy(fullName, dir);
                int len = strlen(dir);
                fullName[len++] = '/';
                strcpy(fullName + len, fileName);
                directoryEntries[entryNum] = fullName;
                dirLogicalClusters[entryNum] = dirEntry.DE_FirstLogicalCluster;
                flags[entryNum] = ENTRY_DIRECTORY;
            }
            else
            {
                directoryEntries[entryNum] = fileName;
                flags[entryNum] = ENTRY_DIR_SPECIAL;
            }
        }
        else
        {
            // file
            directoryEntries[entryNum] = fileName;
            dirLogicalClusters[entryNum] = dirEntry.DE_FirstLogicalCluster;
            flags[entryNum] = ENTRY_FILE;
        }
        entryNum++;
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
    if (nextLogicalClusterValue >= 0x0FF8 && nextLogicalClusterValue <= 0x0FFF)
    {
        lsPrint(fat12, bootSector, hasParam, dir, directoryEntries, dirLogicalClusters, flags, entryNum);

        // now there are no clusters left, continue to scan the subdirectories one by one
        for (size_t i = 0; i < entryNum; i++)
        {
            if (flags[i] == ENTRY_DIRECTORY)
            {
                char *l_directoryEntries[224];
                int l_dirLogicalClusters[224];
                int l_flags[224];
                ls(fat12, bootSector, hasParam, directoryEntries[i], dirLogicalClusters[i], l_directoryEntries, l_dirLogicalClusters, l_flags, 0);
            }
        }
    }
    else if (nextLogicalClusterValue > 0x001 && nextLogicalClusterValue < 0xFF0)
    {
        // ok, current cluster is not the last
        ls(fat12, bootSector, hasParam, dir, nextLogicalClusterValue, directoryEntries, dirLogicalClusters, flags, entryNum);
    }
}

// example below works, and the　redundant will be ignored
// "       ls -l         nju.txt       -ll         "
int splits(char *src, char *target[], char token)
{
    int dirEntryNum = 0;
    int count = 0;
    char *part;
    // trim space
    while (src[dirEntryNum] == token)
        dirEntryNum++;
    if (src[dirEntryNum] == 0)
    {
        return count;
    }

    while (1)
    {
        part = (char *)malloc(sizeof(char) * 100);
        memset(part, 0, 100);

        int ptr = dirEntryNum;
        while (1)
        {
            if (src[ptr] == token)
            {
                target[count++] = part;

                // trim space
                while (src[ptr] == token)
                    ptr++;
                if (src[ptr] == 0)
                {
                    return count;
                }
                else
                {
                    dirEntryNum = ptr;
                }

                break;
            }
            else if (src[ptr] == 0)
            {
                target[count++] = part;
                return count;
            }
            part[ptr - dirEntryNum] = src[ptr];
            ptr++;
        }
    }
    return count;
}
