#include "../include/semaphore.h"
#include "const.h"
#include "proc.h"
#include "global.h"

/**
 * 阻塞当前进程，并且置成等待信号量s状态，移入s信号量的队列，转向进程调度程序 
 */
PUBLIC void sleep(SEMAPHORE *s)
{
    if (s->list == NULL)
    {
        s->list = p_proc_ready;
    }
    else
    {
        PROCESS *p = s->list;
        while (p->next != NULL)
        {
            p = p->next;
        }
        p->next = p_proc_ready;
    }
    // 修改进程状态为等待态，不再参与进程调度
    p_proc_ready->state = STATE_SLEEP;
    p_proc_ready->ticks = 0;
    // 转向进程调度程序
    schedule();
}

/**
 * 从信号量队列中释放一个等待信号s的进程，并转成就绪态，当前进程继续执行
 * 公平策略是：FCFS算法，因此从队首释放等待进程
 */
PUBLIC void wakeup(SEMAPHORE *s)
{
    PROCESS* p = s->list;
    s->list = p->next;
    p->next = NULL;
    // 修改进程状态
    p->state = STATE_READY;
}