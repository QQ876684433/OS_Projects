#include "headers/error.h"

int FLAGS_PATH;

void INIT_PATH_FLAGS(int flag)
{
    FLAGS_PATH = flag;
}

void PATH_SET_FOUND()
{
    FLAGS_PATH = PATH_FOUND;
}

void PATH_FLAGS_CHECK()
{
    switch (FLAGS_PATH)
    {
    case PATH_DIR_NOT_FOUND:
        sprint("\033[31mPath Not Found!\033[0m\n");
        break;

    case PATH_FILE_NOT_FOUND:
        sprint("\033[31mFile Not Found!\033[0m\n");
        break;

    default:
        break;
    }
}

void PATH_SET_DIR_NOT_FOUND()
{
    FLAGS_PATH = PATH_DIR_NOT_FOUND;
}

void PATH_SET_FILE_NOT_FOUND()
{
    FLAGS_PATH = PATH_FILE_NOT_FOUND;
}

void LOAD_BOOT_SECTOR_ERROR()
{
    sprint("failed to load Boot Sector!\n");
    exit(1);
}

void LOCATE_ROOT_DIRECTORIES_ERROR()
{
    sprint("failed to locate Root Directories!");
    exit(1);
}

void READ_ROOT_DIRECTORIES_ERROR()
{
    sprint("failed to read Directory Entry!");
    exit(1);
}

void LOCATE_FAT_TABLES_ERROR()
{
    sprint("failed to locate FAT tables!\n");
    exit(1);
}

void READ_FAT_TABLES_ERROR()
{
    sprint("failed to read FAT tables!\n");
    exit(1);
}

void LS_NO_SUCH_DIR()
{
    sprint("no such directory!\n");
}

void LOCATE_FILE_ERROR()
{
    sprint("failed to locate file!\n");
}

void READ_FILE_ERROR()
{
    sprint("failed to read file!\n");
}

//===================================================
//===================================================

void WRONG_OPERATION()
{
    sprint("\033[31mWrong Operation!\033[0m\n");
}

void LS_WRONG_PARAM()
{
    sprint("\033[31mWrong Parameter!\033[0m\n");
}

void LS_TOO_MANY_PATH()
{
    sprint("\033[31mToo Many Paths!\033[0m\n");
}

void CAT_TOO_MANY_PARAM()
{
    sprint("\033[31mToo Many Parameters!\033[0m\n");
}

void CAT_NOT_ENOUTH_PARAM()
{
    sprint("\033[31mNot Enough Parameters!\033[0m\n");
}
