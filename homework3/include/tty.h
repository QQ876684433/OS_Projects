
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				tty.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_TTY_H_
#define _ORANGES_TTY_H_

#define TTY_IN_BYTES 256 /* tty input queue size */
#define TTY_BUFFER_SIZE 80 * 25
#define MODE_NORMAL 0 /* 正常输入模式 */
#define MODE_SEARCH 1 /* 输入匹配字符串模式 */
#define MODE_MATCH 2  /* 完成匹配模式 */

struct s_console;

/* TTY */
typedef struct s_tty
{
	u32 in_buf[TTY_IN_BYTES]; /* TTY 输入缓冲区 */
	u32 *p_inbuf_head;		  /* 指向缓冲区中下一个空闲位置 */
	u32 *p_inbuf_tail;		  /* 指向键盘任务应处理的键值 */
	int inbuf_count;		  /* 缓冲区中已经填充了多少 */

	struct s_console *p_console;
	u32 mode;
} TTY;

u32 pattern_start;
int is_ctrlz_op;
int ctrlz_index;
int ctrlz_index_search_mode;
u8 ctrlz_stack[100]; /* 撤销功能栈 */

#endif /* _ORANGES_TTY_H_ */
