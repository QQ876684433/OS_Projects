#ifndef COMMAND_H_
#define COMMAND_H_

#include "includes.h"
#include "fat.h"

// tree
void tree(FILE *, struct BootSector *);
// ls
void ls(FILE *, struct BootSector *, char *, int, char *[], int *, int);

#endif