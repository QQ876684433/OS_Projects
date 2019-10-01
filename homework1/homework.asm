	global	main


section .text
main:
    mov rbp, rsp; for correct debugging
    call    print_hint
    call    get_x_y
    ; call    get_x_y_len
    call    add_x_y
    call    mul_x_y

    mov	eax, 60
	xor	rdi, rdi
	syscall

print_hint:
    mov	rax, 1H
	mov	rdi, 1H
	mov	rdx, 22
	mov	rsi, hint
	syscall
    ret

;==================================
; print the value in register: bl =
;==================================
print_ascii:
	mov	[pbuf], bl
	mov	rax, 1H
	mov	rdi, 1H
	mov	rdx, 1H
	mov	rsi, pbuf
	syscall
    ret




;=============================
; read a char from stdin     =
; the input char saved in bl =
;=============================
get_char:
	;use `pbuf` as input buffer
	mov	rax, 0H
	mov	rdi, 0H
	mov	rsi, pbuf
	mov	rdx, 1H	;read one char
	syscall
	mov	bl, [pbuf]	;save char in register `bl`
    ret


get_x_y:
    ; get x
    mov bh, 20H
    mov rbp, x
    get_x_loop:
        call    get_char
        ; break when receiving space key
        cmp bl, bh
        jz  get_y_loop
        mov [rbp], bl
        inc rbp
        jmp get_x_loop

    ; get y
    get_y_loop:
        ; clear more than one spaces between x and y
        call    get_char
        cmp bl, bh
        jz  get_y_loop
        ; write the first non-space character
        mov rbp, y
        mov [rbp], bl
        inc rbp
        ; read the rest of y
        mov	rax, 0H
        mov	rdi, 0H
        mov	rsi, rbp
        mov	rdx, 24
        syscall
    ret


; get_x_y_len:
;     ;============================================
;     ; cal x length
;     mov al, 0H   ; temp for x_len
;     mov rbp, x
;     get_x_len_loop:
;         mov ah, [rbp]
;         inc rbp
;         cmp ah, 0
;         jz end_x_len
;         inc al
;         jmp get_x_len_loop
;     end_x_len:
;         mov byte [x_len], al ; save x_len to memory
;     ;============================================

;     ;============================================
;     ; cal y length
;     mov al, 0H   ; temp for y_len
;     mov rbp, y
;     get_y_len_loop:
;         mov ah, [rbp]
;         inc rbp
;         cmp ah, 0
;         jz end_y_len
;         inc al
;         jmp get_y_len_loop
;     end_y_len:
;         sub al, 1   ; remove the \n or \r
;         mov byte [y_len], al ; save x_len to memory
;     ;============================================

;     ret

add_x_y:
    mov rbx, x  ; save the last index of x
    get_x_end_loop:
        mov ah, [rbx]
        cmp ah, 0
        jz end_x_end
        inc rbx
        jmp get_x_end_loop
    end_x_end:
        dec rbx

    mov rbp, y  ; save the last index of y
    get_y_end_loop:
        mov ah, [rbp]
        cmp ah, 0AH
        jz end_y_end
        inc rbp
        jmp get_y_end_loop
    end_y_end:
        dec rbp

    ; set flag 0
    mov rax, 0
    push    1
    add_x_y_loop:
        mov al, ah  ; add the flag as the initial value
        add_x_to_al:
            cmp rbx, x  ; judge if rbx lower than x in memory
            jb  add_y_to_al ; no need to add x if true
            add al, byte [rbx]
            sub al, 48
        add_y_to_al:
            cmp rbp, y
            jb  cal_flag
            add al, byte [rbp]
            sub al, 48
        cal_flag:
            add al, 48
            call    set_flag
        ; write result
        ; push    rbx
        ; push    rbp
        ; push    rax
        ; mov bl, al
        ; call    print_ascii
        ; pop rax
        ; pop rbp
        ; pop rbx
        push    rax

        dec rbx
        dec rbp
        cmp rbx, x
        jnb add_x_y_loop
        cmp rbp, y
        jnb add_x_y_loop
    
    ; 把最后一个进位加上去
    cmp ah, 1
    jnz print_result_loop
    mov al, 49
    push    rax

    ; print result
    print_result_loop:
        pop rax
        cmp rax, 1
        jz  add_x_y_end
        mov bl, al
        call    print_ascii
        jmp print_result_loop
    add_x_y_end:
        ret

mul_x_y:

    ret


; 设置进位，如果有进位，ah为1，否则为0
set_flag:
    cmp al, 58
    jb  clear_flag
    mov ah, 1
    sub al, 10
    ret
clear_flag:
    mov ah, 0
    ret




section	.data
    hint:	db	"Please input x and y: "
    x_end:  db    0
    y_end:  db    0



section	.bss
    add_result: resb    24
    mul_result: resb    48
    x:  resb    24
    y:  resb    24
    pbuf:	resb	1

