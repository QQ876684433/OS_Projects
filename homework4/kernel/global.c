
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

PUBLIC PROCESS proc_table[NR_TASKS];

PUBLIC char task_stack[STACK_SIZE_TOTAL];

PUBLIC TASK task_table[NR_TASKS] = {{ProcessA, STACK_SIZE_PROCESSA, "ProcessA"},
                                    {ProcessB, STACK_SIZE_PROCESSB, "ProcessB"},
                                    {ProcessC, STACK_SIZE_PROCESSC, "ProcessC"},
                                    {ProcessD, STACK_SIZE_PROCESSD, "ProcessD"},
                                    {ProcessE, STACK_SIZE_PROCESSE, "ProcessE"},
                                    {ProcessF, STACK_SIZE_PROCESSF, "ProcessF"}};

PUBLIC irq_handler irq_table[NR_IRQ];

PUBLIC system_call sys_call_table[NR_SYS_CALL] = {sys_get_ticks, sys_sprint, sys_milli_seconds, sys_P, sys_V, sys_BP, sys_BV};
