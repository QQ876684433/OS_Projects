#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#define IMAGE "images/fat12.img"
#define BOOT_SECTOR_OFFSET 11

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

/**
 * struct for file or directory name
 */
struct FileName
{
    char name[9];
    char ext[4];
};

// global variables
struct BootSector *bootSector;

// 读取启动扇区
void loadBootSector(FILE *, struct BootSector *);
// tree
void tree(FILE *, char *dir);

int main(int argc, char const *argv[])
{
    FILE *fat12 = fopen(IMAGE, "rb");
    struct BootSector bs;

    loadBootSector(fat12, &bs);
    bootSector = &bs;
    printf("每个FAT占用的扇区数:%d\n", bs.BS_SectorsPerFAT);
    printf("保留扇区的数量:%d\n", bs.BS_NumberOfReservedSectors);
    printf("每扇区字节数:%d\n", bs.BS_BytesPerSector);
    printf("每簇扇区数:%d\n", bs.BS_SectorsPerCluster);
    printf("Boot记录占用扇区数:%d\n", bs.BS_NumberOfReservedSectors);
    printf("共有FAT表数:%d\n", bs.BS_NumberOfFATs);
    printf("根目录文件数最大值:%d\n", bs.BS_MaxNumOfRootDirectoryEntries);
    printf("一个FAT占用扇区数:%d\n", bs.BS_SectorsPerFAT);

    tree(fat12, NULL);
    return 0;
}

void loadBootSector(FILE *fat12, struct BootSector *bs_ptr)
{
    /**
     * int fseek(FILE *fp, long offset, int origin)：fseek()用于将文件指针指向任意位置
     * offset为偏移量，即要移动的字节数
     * origin为起始位置，也就是从何处计算偏移量，有三种取值SEEK_SET文件开头、SEEK_CUR当前位置、SEEK_END文件末尾，这是三个常量，值分别为 0、1、2
     * 如果指针移动成功，返回0值，移动失败，则不改变指针的位置，返回非0值 
     */
    int index = fseek(fat12, BOOT_SECTOR_OFFSET, SEEK_SET);
    if (index != 0)
    {
        printf("failed to load Boot Sector!\n");
        exit(1);
    }

    /**
     * size_t fread(void *ptr, size_t size, size_t count, FILE *fp);
     * ptr是内存区块的指针，可以是任何类型的数据，在fread()中用来存放读取的数据，在fwrite()中用来存放要被写入的数据
     * size表示每次读取/写入的长度，单位为字节
     * count表示总共读取/写入的次数
     * fp为文件指针
     */
    index = fread(bs_ptr, 1, sizeof(struct BootSector), fat12);
    if (index != sizeof(struct BootSector))
    {
        printf("failed to load Boot Sector!\n");
        exit(1);
    }
}

void tree(FILE *fat12, char *dir)
{
    if (dir == NULL)
    {
        dir = "/";
    }

    struct DirEntry dirEntry;
    struct DirEntry *de_ptr = &dirEntry;
    int base =
        (bootSector->BS_NumberOfReservedSectors + bootSector->BS_NumberOfFATs * bootSector->BS_SectorsPerFAT) * bootSector->BS_BytesPerSector;

    struct FileName fileName;
    for (size_t i = 0; i < bootSector->BS_MaxNumOfRootDirectoryEntries; i++)
    {
        int index = fseek(fat12, base, SEEK_SET);
        if (index != 0)
        {
            printf("failed to locate Root Directories (i = %d)!", i);
            exit(1);
        }

        index = fread(de_ptr, 1, 32, fat12);
        if (index != 32)
        {
            printf("failed to read Directory Entry (i = %d)!", i);
            exit(1);
        }

        // point to next Directory Entry
        base += 32;

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
        struct FileName fileName;
        memset(&fileName, 0, sizeof(struct FileName));
        for (size_t j = 0; j < 8; j++)
        {
            if (dirEntry.DE_Filename[j] == ' ')
            {
                break;
            }
            fileName.name[j] = dirEntry.DE_Filename[j];
        }
        for (size_t j = 0; j < 3; j++)
        {
            if (dirEntry.DE_Filename[8 + j] == ' ')
            {
                break;
            }
            fileName.ext[j] = dirEntry.DE_Filename[8 + j];
        }

        // skip hidden file or directories
        if ((dirEntry.DE_Attributes & 0x02) != 0)
        {
            continue;
        }

        if ((dirEntry.DE_Attributes & 0x10) != 0)
        {
            // SubDirectory
            printf("[DIR]\t%s.%s\n", fileName.name, fileName.ext);
        }
        else
        {
            // file
            printf("[FILE]\t%s.%s\n", fileName.name, fileName.ext);
        }
    }
}
