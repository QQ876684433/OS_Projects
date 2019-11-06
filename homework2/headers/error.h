#ifndef ERROR_H_
#define ERROR_H_

#include "includes.h"

// 判断路径是否存在的标志位
#define PATH_FOUND 1
#define PATH_FILE_NOT_FOUND 2
#define PATH_DIR_NOT_FOUND 3

void INIT_PATH_FLAGS(int);

void PATH_SET_FOUND();

void PATH_FLAGS_CHECK();

void PATH_SET_DIR_NOT_FOUND();

void PATH_SET_FILE_NOT_FOUND();

void LOAD_BOOT_SECTOR_ERROR();

void LOCATE_ROOT_DIRECTORIES_ERROR();

void READ_ROOT_DIRECTORIES_ERROR();

void LOCATE_FAT_TABLES_ERROR();

void READ_FAT_TABLES_ERROR();

void LS_NO_SUCH_DIR();

void LOCATE_FILE_ERROR();

void READ_FILE_ERROR();

// command error messages
void WRONG_OPERATION();

void LS_WRONG_PARAM();

void LS_TOO_MANY_PATH();

void CAT_TOO_MANY_PARAM();

void CAT_NOT_ENOUTH_PARAM();

#endif