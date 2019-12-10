
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_sprint			equ	1
_NR_milli_seconds	equ	2
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global	sprint
global	milli_seconds

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret
	
; ====================================================================
;                              sprint
; ====================================================================
sprint:
	mov	eax, _NR_sprint
	; 注意，这里不能使用ebx传参，不然会导致页错误，不知道为什么
	mov	ecx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                              milli_seconds
; ====================================================================
milli_seconds:
	mov	eax, _NR_milli_seconds
	mov	ecx, [esp + 4]
	int	INT_VECTOR_SYS_CALL	
	ret
