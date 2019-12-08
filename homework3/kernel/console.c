
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


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

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
	/* 初始化line变量，用来纪录每行最后个字符的位置，以方便删除时退格到上一行的正确位置 */
	for (int i = 0; i < 1000; i++)
	{
		p_tty->p_console->lines[i] = 79;
	}

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		clear_screen(p_tty);
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
		set_cursor(p_tty->p_console->cursor);
	}
}

/*======================================================================*
			   clear_screen
 *======================================================================*/
PUBLIC void clear_screen(TTY *p_tty)
{
	// unsigned int disp = p_tty->p_console->cursor;
	// while (disp--)
	// {
	// 	out_char(p_tty->p_console, '\b');
	// }
	u8 *p_vmem = (u8 *)(V_MEM_BASE + p_tty->p_console->original_addr * 2);
	for (unsigned int i = 0; i < p_tty->p_console->v_mem_limit; i++)
	{
		*p_vmem++ = ' ';
		*p_vmem++ = DEFAULT_CHAR_COLOR;
	}

	for (int i = 0; i < 1000; i++)
	{
		p_tty->p_console->lines[i] = 0;
	}
	
	p_tty->p_console->cursor = p_tty->p_console->original_addr;
	set_cursor(p_tty->p_console->cursor);
}

/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE *p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	/* 根据当前的tty模式获取文本颜色 */
	u8 OUTPUT_CHAR_COLOR = 
		tty_table[nr_current_console].mode == 
			MODE_NORMAL ? DEFAULT_CHAR_COLOR : SEARCH_MODE_CHAR_COLOR;

	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case CTRL_Z:
		if (tty_table[nr_current_console].mode == MODE_MATCH)
		{
			break;
		}
		if (ctrlz_index > /* 匹配模式下的栈底指针是ctrlz_index_search_mode，普通模式下是0 */
			(tty_table[nr_current_console].mode == MODE_SEARCH ? ctrlz_index_search_mode : 0))
		{
			is_ctrlz_op = 1;
			out_char(p_con, ctrlz_stack[--ctrlz_index]);
			is_ctrlz_op = 0;
		}
		break;
	case '\t':
		if (tty_table[nr_current_console].mode == MODE_MATCH)
		{
			break;
		}
		if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 4)
		{
			for(unsigned int i = 0; i < 4; i++)
			{
				*p_vmem++ = 0;
				*p_vmem++ = OUTPUT_CHAR_COLOR;
				p_con->cursor++;
			}
			unsigned int line = p_con->cursor / 80;
			unsigned int pos = p_con->cursor % 80;
			p_con->lines[line] = pos;
		}
		if (is_ctrlz_op != 1)
		{
			ctrlz_stack[ctrlz_index++] = '\b';
		}
		break;
	case '\n':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
		}
		if (is_ctrlz_op != 1)
		{
			ctrlz_stack[ctrlz_index++] = '\b';
		}
		break;
	case '\b':
		if (tty_table[nr_current_console].mode == MODE_MATCH)
		{
			break;
		}
		else if (tty_table[nr_current_console].mode == MODE_SEARCH)
		{
			/* 匹配模式不能删除之前输入的值 */
			if (p_con->cursor <= pattern_start)
			{
				break;
			}
		}
		
		if (p_con->cursor > p_con->original_addr) {
			// *******************************************************************
			// 如果要删除的字符的ASCII码是0，说明遇到了tab，因此直接将光标回退4位即可
			// 关键在于换行的处理
			// *******************************************************************
			unsigned int line = p_con->cursor / 80;
			unsigned int pos = p_con->cursor % 80;
			if (*(p_vmem-2) == 0) 
			{
				if (pos < 4)
				{
					line--;
					p_con->cursor = line * 80 + p_con->lines[line];
				}
				else
				{
					p_con->cursor -= 4;
					p_con->lines[line] = pos - 4;
				}

				// 删除tab的逆操作
				if (is_ctrlz_op != 1)
				{
					ctrlz_stack[ctrlz_index++] = '\t';
				}
			}
			else
			{
				int isNL = 0;	// 用来记录当前删除的是不是换行符
				if (pos == 0)
				{
					isNL = 1;	// 说明当前删除的是换行
					p_con->cursor = (line -1) * 80 + p_con->lines[line - 1];
				}
				else
				{
					p_con->cursor--;
				}

				// 删除tab之外的字符的撤销
				if (is_ctrlz_op != 1)
				{
					ctrlz_stack[ctrlz_index++] = isNL ? '\n' : *(p_vmem - 2);
				}

				*(p_vmem - 2) = ' ';
				*(p_vmem - 1) = OUTPUT_CHAR_COLOR;

				unsigned int line = p_con->cursor / 80;
				unsigned int pos = p_con->cursor % 80;
				p_con->lines[line] = pos;
			}
		}
		break;
	default:
		if (tty_table[nr_current_console].mode == MODE_MATCH)
		{
			break;
		}
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = OUTPUT_CHAR_COLOR;
			p_con->cursor++;

			unsigned int line = p_con->cursor / 80;
			unsigned int pos = p_con->cursor % 80;
			p_con->lines[line] = pos;
		}
		if (is_ctrlz_op != 1)
		{
			ctrlz_stack[ctrlz_index++] = '\b';
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
	if (is_current_console(p_con)) {
		set_cursor(p_con->cursor);
		set_video_start_addr(p_con->current_start_addr);
	}
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	flush(&console_table[nr_console]);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	flush(p_con);
}

