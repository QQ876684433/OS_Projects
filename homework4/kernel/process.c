#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

// 定义作业任务宏
#define _reader_first_with_writer_hungry    // 读者优先，不解决写者饥饿问题
#define _reader_first_without_writer_hungry // 读者优先，通过每次读者读完即 \
											// milli_seconds(一个时间片)来解决写者饥饿

/*===================================================================================*
									题目要求
	– 共有 6 个一直存在的进程（循环读写操作）， A、 B、 C 为读者进程， D、 E 为写者进
	程， F 为普通进程，其中
	∗ A 阅读消耗 2 个时间片
	∗ B、 C 阅读消耗 3 个时间片
	∗ D 写消耗 3 个时间片
	∗ E 写消耗 4 个时间片
	– 读者在读的时候，写者不能写，必须等到全部读者读完
	– 同时只能一个作者在写
	– 在写的时候，读者不能读
	– 多个读者可以读一本书，但是不能太多，上限数字有 1、 2、 3，需要都能够支持，并
	且可以现场修改
	– A、 B、 C、 D、 E 进程需要彩色打印基本操作：读开始、写开始、读、写、读完成、写
	完成，以及对应进程名字
	– F 每隔 1 个时间片打印当前是读还是写，如果是读有多少人
	– 请分别实现读者优先和写者优先，需要都能够支持，并且可以现场修改
	– 请想办法解决此问题中部分情况下的进程饿死问题 (可参考第六章)
 *===================================================================================*/

/*===================================================================================*
									进程列表
					共有 6 个一直存在的进程（循环读写操作）， 
					A、 B、 C 为读者进程， D、 E 为写者进程， F 为普通进程
 *===================================================================================*/

void ProcessA()
{
    int cost = 2 * time_slice;
    char *start = "A begin reading  ";
    char *reading = "A reading  ";
    char *end = "A end reading  ";
    int color = 1;
    while (1)
    {
        B_P(&mutex);
        readcount++;
        if (readcount == 1)
        {
            B_P(&writeblock);
        }
        readerNum++;
        disp_color_str(start, color);
        B_V(&mutex);

        // 读文件
        disp_color_str(reading, color);
        milli_delay(cost);

        B_P(&mutex);
        readcount--;
        if (readcount == 0)
        {
            B_V(&writeblock);
        }
        disp_color_str(end, color);
        readerNum--;
        B_V(&mutex);

#ifdef _reader_first_without_writer_hungry
        milli_seconds(1000);
#endif
    }
}

void ProcessB()
{
    int cost = 3 * time_slice;
    char *start = "B begin reading  ";
    char *reading = "B reading  ";
    char *end = "B end reading  ";
    int color = 2;
    while (1)
    {
        B_P(&mutex);
        readcount++;
        if (readcount == 1)
        {
            B_P(&writeblock);
        }
        readerNum++;
        disp_color_str(start, color);
        B_V(&mutex);

        // 读文件
        disp_color_str(reading, color);
        milli_delay(cost);

        B_P(&mutex);
        readcount--;
        if (readcount == 0)
        {
            B_V(&writeblock);
        }
        disp_color_str(end, color);
        readerNum--;
        B_V(&mutex);

#ifdef _reader_first_without_writer_hungry
        milli_seconds(1000);
#endif
    }
}

void ProcessC()
{
    int cost = 3 * time_slice;
    char *start = "C begin reading  ";
    char *reading = "C reading  ";
    char *end = "C end reading  ";
    int color = 3;
    while (1)
    {
        B_P(&mutex);
        readcount++;
        if (readcount == 1)
        {
            B_P(&writeblock);
        }
        readerNum++;
        disp_color_str(start, color);
        B_V(&mutex);

        // 读文件
        disp_color_str(reading, color);
        milli_delay(cost);

        B_P(&mutex);
        readcount--;
        if (readcount == 0)
        {
            B_V(&writeblock);
        }
        disp_color_str(end, color);
        readerNum--;
        B_V(&mutex);

#ifdef _reader_first_without_writer_hungry
        milli_seconds(1000);
#endif
    }
}

void ProcessD()
{
    int cost = 3 * time_slice;
    char *start = "D begin writing  ";
    char *writing = "D writing  ";
    char *end = "D end writing  ";
    int color = 4;
    while (1)
    {
        B_P(&writeblock);
        writerNum++;
        disp_color_str(start, color);

        // 写文件
        disp_color_str(writing, color);
        milli_delay(cost);

        disp_color_str(end, color);
        writerNum--;
        B_V(&writeblock);
    }
}

void ProcessE()
{
    int cost = 4 * time_slice;
    char *start = "E begin writing  ";
    char *writing = "E writing  ";
    char *end = "E end writing  ";
    int color = 5;
    while (1)
    {
        B_P(&writeblock);
        writerNum++;
        disp_color_str(start, color);

        // 写文件
        disp_color_str(writing, color);
        milli_delay(cost);

        disp_color_str(end, color);
        writerNum--;
        B_V(&writeblock);
    }
}

void ProcessF()
{
    int cost = 1 * time_slice;
    while (1)
    {
        // B_P(&mutex);
        milli_seconds(cost);
        if (!writerNum)
        {
            sprint("[now is reading: ");
            disp_int(readerNum);
            sprint("]  ");
        }
        else
        {
            sprint("[now is writing]  ");
        }
        // B_V(&mutex);
    }
}
