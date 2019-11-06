#ifndef COMMAND_H_
#define COMMAND_H_

#include "includes.h"
#include "fat.h"

#define EXIT "exit"
#define CAT "cat"
#define LS "ls"

#define LS_WITH_PARAM 1
#define LS_NO_PARAM 0

#define ENTRY_DIRECTORY 1
#define ENTRY_FILE 0
#define ENTRY_DIR_SPECIAL 2

/**
 * command function
 */

// tree
void tree(FILE *, struct BootSector *, int, char *);
// ls
void ls(FILE *, struct BootSector *, int, char *, char *, int, char *[], int *, int *, int);

/**
 * util function
 */

// split command into several parts separated by space, return the number of parts
int splits(char *, char *[], char);
//
void countDirAndFile(FILE *fat12, struct BootSector *bootSector, int logicalCluster, int *flags, int entryNum);
//
void lsPrint(FILE *fat12, struct BootSector *bootSector, int hasParam, char *dir, char *directoryEntries[], int dirLogicalClusters[], int *flags, int entryNum);
// 
int isDirPrefixMatch(char *, char *);

#endif