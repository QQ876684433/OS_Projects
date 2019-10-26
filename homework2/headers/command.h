#ifndef COMMAND_H_
#define COMMAND_H_

#include "includes.h"
#include "fat.h"

#define EXIT "exit"
#define CAT "cat"
#define LS "ls"

/**
 * command function
 */

// tree
void tree(FILE *, struct BootSector *);
// ls
void ls(FILE *, struct BootSector *, char *, int, char *[], int *, int);

/**
 * util function
 */

// split command into several parts separated by space, return the number of parts
int splits(char *, char *[]);

#endif