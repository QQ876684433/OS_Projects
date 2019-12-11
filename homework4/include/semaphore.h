#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "proc.h"
#include "const.h"

typedef struct s_semaphore
{
    int value;     /* 信号量值 */
    PROCESS *list; /* 信号量队列指针 */
} SEMAPHORE;

PUBLIC void sleep(SEMAPHORE *s);
PUBLIC void wakeup(SEMAPHORE *s);

#endif