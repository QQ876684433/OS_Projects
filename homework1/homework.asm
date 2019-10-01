	global	main


section .text
main:
    mov rbp, rsp; for correct debugging
    call    print_hint
    call    get_x_y
    call    add_x_y
    mov bl, 10
    call    print_ascii
    call    mul_x_y
    mov bl, 10
    call    print_ascii

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


;==============================================
; get x and y from stdin separated with space =
;==============================================
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


;========================================================
; get end of x and y, saved in rbx and rbp respectively =
;========================================================
get_x_y_end:
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
    ret

add_x_y:
    call    get_x_y_end

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

;================================
; result saved in al            =
; if y==0, al will be 1, else 0 =
;================================
check_y_equal_zero:
    push    rbp
    mov al, 1
    check_loop:
        mov ah, byte [rbp]
        cmp ah, 48
        jnz not_equal_zero
        dec rbp
        cmp rbp, y
        jb  end_check_loop
        jmp check_loop
    end_check_loop:
        pop rbp
        ret
    not_equal_zero:
        mov al, 0
        jmp end_check_loop

sub_1_from_y:
    push    rbp
    sub_loop:
        cmp byte [rbp], 48
        jz  left_shift
        dec byte [rbp]
        jmp end_sub_loop
        left_shift:
            mov byte [rbp], 57
            dec rbp
            jmp sub_loop
    end_sub_loop:
        pop rbp
        ret

mul_x_y:
    call    get_x_y_end
    mov byte [mul_result], '0'    
    
    mul_x_y_loop:
        call    check_y_equal_zero
        cmp al, 1
        jz  end_of_mul_x_y
        call    sub_1_from_y
        push    rbx

        ; set flag 0
        mov rax, 0
        mov rcx, mul_result
        add_x_to_mul_result_loop:
            mov al, ah  ; add the flag as the initial value
            add al, byte [rcx]
            cmp rbx, x
            jb  set_result_flag
            add al, byte [rbx]
            sub al, 48
            set_result_flag:
                call    set_flag
            mov byte [rcx], al
            inc rcx
            dec rbx
            cmp rbx, x
            jnb add_x_to_mul_result_loop
        add_rest_flag:
            cmp ah, 0
            jz  end_add_rest_flag
            mov al, ah
            add al, byte [rcx]
            call    set_flag
            mov byte [rcx], al
            inc rcx
            jmp add_rest_flag
        end_add_rest_flag:    
            pop rbx
            jmp mul_x_y_loop

    end_of_mul_x_y:
        ; mov	rax, 1H
        ; mov	rdi, 1H
        ; mov	rdx, 48
        ; mov	rsi, mul_result
        ; syscall
        mov rax, 0
        mov rbx, mul_result
        push    1   ; stack buttom
        push_loop:
            mov cl, byte [rbx]
            push    rcx
            inc rbx
            inc rax
            cmp rax, 48
            jnz push_loop
        pop_zero_loop:
            pop rax
            cmp al, 48
            jz  pop_zero_loop
            mov bl, al
            call    print_ascii
        print_rest_char:
            pop rax
            cmp rax, 1
            jz  end_print_rest_char
            mov bl, al
            call    print_ascii
            jmp print_rest_char
    end_print_rest_char:
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
    mul_result: times 48    db    48



section	.bss
    x:  resb    24
    y:  resb    24
    pbuf:	resb	1

