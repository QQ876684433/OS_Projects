
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY *p_tty);
PRIVATE void tty_do_read(TTY *p_tty);
PRIVATE void tty_do_write(TTY *p_tty);
PRIVATE void put_key(TTY *p_tty, u32 key);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY *p_tty;

	init_keyboard();

	for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
	{
		init_tty(p_tty);
	}
	select_console(0);
	while (1)
	{
		for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
		{
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY *p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->mode = MODE_NORMAL;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY *p_tty, u32 key)
{
	if (!(key & FLAG_EXT))
	{
		put_key(p_tty, key);
	}
	else
	{
		int raw_code = key & MASK_RAW;
		switch (raw_code)
		{
		case ESC:
			if (p_tty->mode == MODE_NORMAL)
			{
				start_search();
			}else
			{
				end_search();
			}
			break;
		case TAB:
			put_key(p_tty, '\t');
			break;
		case ENTER:
			if (p_tty->mode == MODE_NORMAL)
			{
				put_key(p_tty, '\n');
			}else if (p_tty->mode == MODE_SEARCH)
			{
				/* 开始匹配ESC模式下输入的查找串 */
				start_match();
			} else
			{
				// p_tty->mode == MODE_MATCH, do nothing
			}
			break;
		case BACKSPACE:
			put_key(p_tty, '\b');
			break;
		case UP:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
			{
				scroll_screen(p_tty->p_console, SCR_DN);
			}
			break;
		case DOWN:
			if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R))
			{
				scroll_screen(p_tty->p_console, SCR_UP);
			}
			break;
		case F1:
		case F2:
		case F3:
		case F4:
		case F5:
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* Alt + F1~F12 */
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R))
			{
				select_console(raw_code - F1);
			}
			break;
		default:
			break;
		}
	}
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY *p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES)
	{
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES)
		{
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}

/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY *p_tty)
{
	if (is_current_console(p_tty->p_console))
	{
		keyboard_read(p_tty);
	}
}

/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY *p_tty)
{
	if (p_tty->inbuf_count)
	{
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES)
		{
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY *p_tty, char *buf, int len)
{
	char *p = buf;
	int i = len;

	while (i)
	{
		out_char(p_tty->p_console, *p++);
		i--;
	}
}

/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char *buf, int len, PROCESS *p_proc)
{
	tty_write(&tty_table[p_proc->nr_tty], buf, len);
	return 0;
}

/*======================================================================*
                              tty输入模式切换
*======================================================================*/

PUBLIC void start_search(){
	TTY *p_tty = &tty_table[nr_current_console];
	pattern_start = p_tty->p_console->cursor;

	p_tty->mode = MODE_SEARCH;
}

PUBLIC void start_match(){
	TTY *p_tty = &tty_table[nr_current_console];
	// ----------------------------------------------------------------------------
	// 								匹配字符串，Sunday算法
	// ----------------------------------------------------------------------------
	unsigned int index = 0;
	unsigned int str_len = p_tty->p_console->cursor - p_tty->p_console->original_addr;	// 匹配串长度
	unsigned int pat_len = p_tty->p_console->cursor - pattern_start;	// 模式串长度
	u8 *pattern_vmem = (u8 *)(V_MEM_BASE + pattern_start * 2);	// 匹配串起始地址
	u8 *str_vmem = (u8 *)(V_MEM_BASE + p_tty->p_console->original_addr * 2);	// 模式串起始地址
	while (index < str_len)
	{
		int is_found = 1;
		for (unsigned int i = 0; i < pat_len; i++)
		{ 
			if (*(str_vmem + (index + i) * 2)!=*(pattern_vmem + i * 2))
			{
				is_found = 0;
				break;
			}
		}
		if (is_found == 1)	/* 匹配到字符串 */
		{
			unsigned int old_index = index;
			index += pat_len;
			while (old_index < index)
			{
				*(str_vmem + (old_index) * 2 + 1) = SEARCH_MODE_CHAR_COLOR;
				old_index++;
			}
		}else
		{
			if (index + pat_len >= str_len)
			{
				break;
			}
			else
			{
				char ch = *(str_vmem + (index + pat_len) * 2);
				int j;
				for (j = pat_len - 1; j >= 0; j--)
				{
					if (ch == *(pattern_vmem + j * 2))
					{
						break;
					}
				}
				if (j < 0)
				{
					// 匹配串下一个字符不在模式串中
					index += (pat_len + 1);
				}
				else
				{
					index += (pat_len - j);
				}
			}
		}
	}

	p_tty->mode = MODE_MATCH;
}

PUBLIC void end_search(){
	TTY *p_tty = &tty_table[nr_current_console];
	p_tty->mode = MODE_NORMAL;
	// 清除匹配字符串并恢复字符显示颜色
	for (u32 ptr = p_tty->p_console->original_addr; ptr < p_tty->p_console->cursor; ptr++)
	{
		u8 *p_vmem = (u8 *)(V_MEM_BASE + ptr * 2);
		*(p_vmem + 1) = DEFAULT_CHAR_COLOR;
	}
	while (p_tty->p_console->cursor > pattern_start)
	{
		out_char(p_tty->p_console, '\b');
	}
}
