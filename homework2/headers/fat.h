#ifndef FAT_H_
#define FAT_H_

#include "includes.h"
#include "error.h"

#define BOOT_SECTOR_OFFSET 11
#define DIR_ENTRY_PER_SECTOR 16

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#pragma pack(1)

/**
 * The Boot Sector struct
 */
struct BootSector
{
    // ignore 11 bytes

    u16 BS_BytesPerSector;               // 每个扇区的字节数
    u8 BS_SectorsPerCluster;             // 每簇扇区数
    u16 BS_NumberOfReservedSectors;      // 保留扇区数（包含启动扇区）
    u8 BS_NumberOfFATs;                  // 文件分配表FAT数目
    u16 BS_MaxNumOfRootDirectoryEntries; // 最大根目录条目个数
    u16 BS_TotalSectorCount;             // 总扇区数
    u8 BS_Media;
    u16 BS_SectorsPerFAT;            // 每个FAT扇区数
    u16 BS_SectorsPerTrack;          // 每个磁道的扇区数
    u16 BS_NumberOfHeads;            // 磁头数
    u32 BS_HiddenSectors;            // 隐藏扇区
    u32 BS_TotalSectorCountForFAT32; // 0 for FAT12 and FAT16

    // ignore rest of boot sector
};

/**
 * Root Directory Entry
 */
struct DirEntry
{
    u8 DE_Filename[11]; // name(8 bytes) + extension(3 bytes)
    /**
     *  Bit Mask    Attribute
     *  0   0x01    Read-only
     *  1   0x02    Hidden
     *  2   0x04    System
     *  3   0x08    Volume label
     *  4   0x10    Subdirectory
     *  5   0x20    Archive
     *  6   0x40    Unused
     *  7   0x80    Unused
     */
    u8 DE_Attributes;
    u16 DE_Reserved;
    u16 DE_CreationTime;
    u16 DE_CreationDate;
    u16 DE_LastAccessDate;
    u16 DE_Ignored;
    u16 DE_LastWriteTime;
    u16 DE_LastWriteDate;
    u16 DE_FirstLogicalCluster; //开始逻辑簇号
    u32 DE_FileSize;            // 单位是字节
};

#include "command.h"

/**
 * struct for file or directory name
 */
struct FileName
{
    char name[9];
    char ext[4];
};

// 读取启动扇区
void loadBootSector(FILE *, struct BootSector *);
// map directory entry to struct
void mapDirEntry(FILE *, struct DirEntry *, int);
// combine file or subdirectory name and  extension
char *extractFileName(char *fileName);
// given logical cluster value, calculate the base of the physical sector
int getPhysicalBase(int, struct BootSector *);
// given the current logical cluster value, get next logical cluster value, return 1 if successful and 0 otherwise
u16 getNextLogicalCluster(FILE *, int, struct BootSector *);
// show basic information in boot sector
void showBootSectorInfo(struct BootSector *);
// get all Root Directory Entries list
void getRootDirectories(FILE *, struct BootSector *, char *[], int *, int *, int *);
// get non root Directory Entries list
void getNonRootDirectories(FILE *, struct BootSector *, int, char *, char *[], int *, int *, int *);

// Directory Entry Attribute Util Functions
int isHidden(unsigned int attr);
int isSubDirectory(unsigned int attr);

#endif