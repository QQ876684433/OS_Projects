#include "headers/command.h"

void tree(FILE *fat12, struct BootSector *bootSector, int hasParam, char *target)
{
    char *directoryEntries[224];
    int logicalClusters[224];
    int flags[224];
    int entryNum = 0;
    getRootDirectories(fat12, bootSector, directoryEntries, logicalClusters, flags, &entryNum);
    if (isDirPrefixMatch(target, "") == 1)
        lsPrint(fat12, bootSector, hasParam, "", directoryEntries, logicalClusters, flags, entryNum);

    for (size_t i = 0; i < entryNum; i++)
    {
        if (flags[i] == ENTRY_DIRECTORY)
        {
            char *l_directoryEntries[224];
            int l_dirLogicalClusters[224];
            int l_flags[224];
            ls(fat12, bootSector, hasParam, directoryEntries[i], target, logicalClusters[i], l_directoryEntries, l_dirLogicalClusters, l_flags, 0);
        }
    }
}

void ls(FILE *fat12, struct BootSector *bootSector, int hasParam, char *dir, char *target, int logicalCluster, char *directoryEntries[], int *dirLogicalClusters, int *flags, int entryNum)
{
    getNonRootDirectories(fat12, bootSector, logicalCluster, dir, directoryEntries, dirLogicalClusters, flags, &entryNum);

    // add dir perfix to directory entries
    for (size_t i = 0; i < entryNum; i++)
    {
        if (flags[i] == ENTRY_DIRECTORY)
        {
            char *fullName = (char *)malloc(100);
            memset(fullName, 0, 100);
            strcpy(fullName, dir);
            int len = strlen(dir);
            fullName[len++] = '/';
            strcpy(fullName + len, directoryEntries[i]);
            directoryEntries[i] = fullName;
        }
    }
    if (isDirPrefixMatch(target, dir) == 1)
    {
        lsPrint(fat12, bootSector, hasParam, dir, directoryEntries, dirLogicalClusters, flags, entryNum);
    }

    // now there are no clusters left, continue to scan the subdirectories one by one
    for (size_t i = 0; i < entryNum; i++)
    {
        if (flags[i] == ENTRY_DIRECTORY)
        {
            char *l_directoryEntries[224];
            int l_dirLogicalClusters[224];
            int l_flags[224];
            ls(fat12, bootSector, hasParam, directoryEntries[i], target, dirLogicalClusters[i], l_directoryEntries, l_dirLogicalClusters, l_flags, 0);
        }
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

void lsPrint(FILE *fat12, struct BootSector *bootSector, int hasParam, char *dir, char *directoryEntries[], int dirLogicalClusters[], int *flags, int entryNum)
{
    if (hasParam == LS_NO_PARAM)
    {
        printf("%s/:\n", dir);
        for (size_t i = 0; i < entryNum; i++)
        {
            char *sp[224];
            int count = splits(directoryEntries[i], sp, '/');
            if (flags[i] == ENTRY_FILE)
            {
                printf("%s  ", sp[count - 1]);
            }
            else
            {
                printf("\033[31m%s\033[0m  ", sp[count - 1]);
            }
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

        printf("%s/ %d %d:\n", strcmp(dir, "") == 0 ? "" : dir, dn, fn);
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
                printf("\033[31m%s\033[0m %d %d\n", sp[count - 1], dirNum, fileNum);
            }
            else if (flags[i] == ENTRY_DIR_SPECIAL)
            {
                printf("\033[31m%s\033[0m\n", directoryEntries[i]);
            }
            else
            {
                printf("%s\n", directoryEntries[i]);
            }
        }
    }
    printf("\n");
}

int isDirPrefixMatch(char *prefix, char *dir)
{
    int prefixLen = strlen(prefix);
    for (size_t i = 0; i < prefixLen; i++)
    {
        if (dir[i] != prefix[i])
        {
            return 0;
        }
    }
    return 1;
}
