#include "headers/command.h"

void tree(FILE *fat12, struct BootSector *bootSector, int hasParam, char *target, int operation)
{
    char *directoryEntries[224];
    int logicalClusters[224];
    int flags[224];
    int entryNum = 0;
    getRootDirectories(fat12, bootSector, directoryEntries, logicalClusters, flags, &entryNum);
    if (operation == LS_WITH_CAT)
    {
        for (size_t i = 0; i < entryNum; i++)
        {
            // let examples below make sense
            //
            // >cat TEST1.TXT
            // this is a test file for fat12
            // >cat /TEST1.TXT
            // this is a test file for fat12
            char *path = (char *)malloc(100);
            memset(path, 0, 100);
            path[0] = '/';
            strcat(path, directoryEntries[i]);

            if (flags[i] == ENTRY_FILE && strcmp(path, target) == 0)
            {
                // 将路径标志为存在
                PATH_SET_FOUND();
                // print file content
                cat(fat12, bootSector, logicalClusters[i], LS_WITH_CAT);
                return;
            }
        }
    }
    else if (isDirPrefixMatch(target, "") == 1)
    {
        // 将路径标志为存在
        PATH_SET_FOUND();
        lsPrint(fat12, bootSector, hasParam, "", directoryEntries, logicalClusters, flags, entryNum);
    }

    for (size_t i = 0; i < entryNum; i++)
    {
        if (flags[i] == ENTRY_DIRECTORY)
        {
            char *l_directoryEntries[224];
            int l_dirLogicalClusters[224];
            int l_flags[224];
            ls(fat12, bootSector, hasParam, directoryEntries[i], target, logicalClusters[i], l_directoryEntries, l_dirLogicalClusters, l_flags, 0, operation);
        }
    }
}

void ls(FILE *fat12, struct BootSector *bootSector, int hasParam, char *dir, char *target, int logicalCluster, char *directoryEntries[], int *dirLogicalClusters, int *flags, int entryNum, int operation)
{
    getNonRootDirectories(fat12, bootSector, logicalCluster, dir, directoryEntries, dirLogicalClusters, flags, &entryNum);

    // add dir prefix to directory entries
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

    if (operation == LS_WITH_CAT)
    {
        for (size_t i = 0; i < entryNum; i++)
        {
            if (flags[i] == ENTRY_FILE)
            {
                char *fullName = (char *)malloc(100);
                memset(fullName, 0, 100);
                strcat(fullName, dir);
                strcat(fullName, "/");
                strcat(fullName, directoryEntries[i]);
                if (strcmp(fullName, target) == 0)
                {
                    // 将路径标志为存在
                    PATH_SET_FOUND();
                    cat(fat12, bootSector, dirLogicalClusters[i], LS_WITH_CAT);
                }
            }
        }
    }
    else if (isDirPrefixMatch(target, dir) == 1)
    {
        // 将路径标志为存在
        PATH_SET_FOUND();
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
            ls(fat12, bootSector, hasParam, directoryEntries[i], target, dirLogicalClusters[i], l_directoryEntries, l_dirLogicalClusters, l_flags, 0, operation);
        }
    }
}

int cat(FILE *fat12, struct BootSector *bootSector, int logicalCluster, int isPrint)
{
    int base = getPhysicalBase(logicalCluster, bootSector);
    u8 *buffer = (u8 *)malloc(bootSector->BS_BytesPerSector);
    memset(buffer, 0, bootSector->BS_BytesPerSector);
    int index = fseek(fat12, base, SEEK_SET);
    if (index != 0)
    {
        LOCATE_FILE_ERROR();
    }

    index = fread(buffer, 1, bootSector->BS_BytesPerSector, fat12);
    if (index != bootSector->BS_BytesPerSector)
    {
        READ_FILE_ERROR();
    }

    // print the content read
    if (isPrint == LS_WITH_CAT)
        // printf("%s", buffer);
        sprint(buffer);
    // calculate file size
    int size = 0;
    for (size_t i = 0; i < bootSector->BS_BytesPerSector; i++)
    {
        if (buffer[i] != 0)
        {
            size++;
        }
    }

    // check if it's the last cluster
    u16 nextLogicalClusterValue = getNextLogicalCluster(fat12, logicalCluster, bootSector);
    if (nextLogicalClusterValue > 0x001 && nextLogicalClusterValue < 0xFF0)
    {
        // ok, current cluster is not the last
        size += cat(fat12, bootSector, nextLogicalClusterValue, isPrint);
    }
    return size;
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
        // printf("%s/:\n", dir);
        sprint(dir);
        sprint("/:\n");
        for (size_t i = 0; i < entryNum; i++)
        {
            char *sp[224];
            int count = splits(directoryEntries[i], sp, '/');
            if (flags[i] == ENTRY_FILE)
            {
                // printf("%s  ", sp[count - 1]);
                sprint(sp[count - 1]);
                sprint("  ");
            }
            else
            {
                // printf("\033[31m%s\033[0m  ", sp[count - 1]);
                sprint("\033[31m");
                sprint(sp[count - 1]);
                sprint("\033[0m  ");
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

        // printf("%s/ %d %d:\n", strcmp(dir, "") == 0 ? "" : dir, dn, fn);
        char str[25];
        sprint(strcmp(dir, "") == 0 ? "" : dir);
        sprint("/ ");
        sprintf(str, "%d", dn);
        sprint(str);
        sprintf(str, "%d", fn);
        sprint(" ");
        sprint(str);
        sprint(":\n");
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

                // printf("\033[31m%s\033[0m  %d %d\n", sp[count - 1], dirNum, fileNum);
                char str[25];
                sprint("\033[31m");
                sprint(sp[count - 1]);
                sprint("\033[0m  ");
                sprintf(str, "%d", dirNum);
                sprint(str);
                sprintf(str, "%d", fileNum);
                sprint(" ");
                sprint(str);
                sprint("\n");
            }
            else if (flags[i] == ENTRY_DIR_SPECIAL)
            {
                // printf("\033[31m%s\033[0m\n", directoryEntries[i]);
                sprint("\033[31m");
                sprint(directoryEntries[i]);
                sprint("\033[0m\n");
            }
            else
            {
                // get file size
                int size = cat(fat12, bootSector, dirLogicalClusters[i], LS_WITHOUT_CAT);
                // printf("%s  %d\n", directoryEntries[i], size);
                char str[25];
                sprint(directoryEntries[i]);
                sprint("  ");
                sprintf(str, "%d", size);
                sprint(str);
                sprint("\n");
            }
        }
    }
    // printf("\n");
    sprint("\n");
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
