#include "headers/fat.h"

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
        LOAD_BOOT_SECTOR_ERROR();
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
        LOAD_BOOT_SECTOR_ERROR();
    }
}

void mapDirEntry(FILE *fat12, struct DirEntry *de_ptr, int base)
{
    int index = fseek(fat12, base, SEEK_SET);
    if (index != 0)
    {
        LOCATE_ROOT_DIRECTORIES_ERROR();
    }

    index = fread(de_ptr, 1, 32, fat12);
    if (index != 32)
    {
        READ_ROOT_DIRECTORIES_ERROR();
    }
}

char *extractFileName(char *fileName)
{
    char *name = (char *)malloc(13);
    memset(name, 0, 13);
    int index = 0;
    for (size_t i = 0; i < 8; i++)
    {
        if (fileName[i] == ' ')
        {
            break;
        }
        name[index++] = fileName[i];
    }
    int hasExt = 0;
    for (size_t i = 8; i < 11; i++)
    {
        if (hasExt == 0 && fileName[i] != ' ')
        {
            hasExt = 1;
            name[index++] = '.';
        }
        if (hasExt == 1)
        {
            name[index++] = fileName[i];
        }
    }

    return name;
}

int getPhysicalBase(int logicalCluster, struct BootSector *bootSector)
{
    return (logicalCluster + 31) * bootSector->BS_BytesPerSector;
}

u16 getNextLogicalCluster(FILE *fat12, int curLogicalCluster, struct BootSector *bootSector)
{
    int startOfFatTables = bootSector->BS_NumberOfReservedSectors * bootSector->BS_BytesPerSector + curLogicalCluster * 3 / 2;

    int type = 0; // 偶数簇
    if (curLogicalCluster % 2 == 1)
    { //奇数簇
        type = 1;
    }

    //fat 默认保留两簇
    u16 nextLogicalCluster;

    int check = fseek(fat12, startOfFatTables, SEEK_SET);
    if (check == -1)
    {
        LOCATE_FAT_TABLES_ERROR();
    }

    check = fread(&nextLogicalCluster, 1, 2, fat12);
    if (check != 2)
    {
        READ_FAT_TABLES_ERROR();
    }

    //u16为short，结合存储的小尾顺序和FAT项结构可以得到
    if (type == 0)
    {
        nextLogicalCluster = (nextLogicalCluster & 0x0fff);
    }
    else
    {
        nextLogicalCluster = (nextLogicalCluster >> 4);
    }
    return nextLogicalCluster;
}

void showBootSectorInfo(struct BootSector *bs)
{
    printf("每个FAT占用的扇区数:%d\n", bs->BS_SectorsPerFAT);
    printf("保留扇区的数量:%d\n", bs->BS_NumberOfReservedSectors);
    printf("每扇区字节数:%d\n", bs->BS_BytesPerSector);
    printf("每簇扇区数:%d\n", bs->BS_SectorsPerCluster);
    printf("Boot记录占用扇区数:%d\n", bs->BS_NumberOfReservedSectors);
    printf("共有FAT表数:%d\n", bs->BS_NumberOfFATs);
    printf("根目录文件数最大值:%d\n", bs->BS_MaxNumOfRootDirectoryEntries);
    printf("一个FAT占用扇区数:%d\n", bs->BS_SectorsPerFAT);
}