#include "headers/error.h"

void LOAD_BOOT_SECTOR_ERROR()
{
    printf("failed to load Boot Sector!\n");
    exit(1);
}

void LOCATE_ROOT_DIRECTORIES_ERROR()
{
    printf("failed to locate Root Directories!");
    exit(1);
}

void READ_ROOT_DIRECTORIES_ERROR()
{
    printf("failed to read Directory Entry!");
    exit(1);
}

void LOCATE_FAT_TABLES_ERROR()
{
    printf("failed to locate FAT tables!\n");
    exit(1);
}

void READ_FAT_TABLES_ERROR()
{
    printf("failed to read FAT tables!\n");
    exit(1);
}
