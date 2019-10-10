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
        set_x_flag:
            cmp bl, '-'
            jnz end_set_x_flag
            mov rax, x_flag
            mov byte [rax], '-'
            jmp get_x_loop
        end_set_x_flag:
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
        set_y_flag:
            cmp bl, '-'
            jnz end_set_y_flag
            mov rax, y_flag
            mov byte [rax], '-'
            jmp get_y_loop
        end_set_y_flag:
        mov rbp, y
        mov [rbp], bl
        inc rbp
        ; read the rest of y
        read_y_rest_loop:
            call    get_char
            cmp bl, 10
            jz  end_get_x_y
            mov byte [rbp], bl
            inc rbp
            jmp read_y_rest_loop
    end_get_x_y:
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
        cmp ah, 0
        jz end_y_end
        inc rbp
        jmp get_y_end_loop
    end_y_end:
        dec rbp
    ret

;========================================================
; get start of x and y, saved in r8 and r9 respectively =
;========================================================
get_x_y_start:
    mov r8, x
    mov r9, y
    call    get_x_y_end
    get_x_start_loop:
        cmp r8, rbx
        jz  get_y_start_loop
        cmp byte [r8], '0'
        jnz get_y_start_loop
        inc r8
        jmp get_x_start_loop
    get_y_start_loop:
        cmp r9, rbp
        jz  end_get_x_y_start
        cmp byte [r9], '0'
        jnz end_get_x_y_start
        inc r9
        jmp get_y_start_loop
    end_get_x_y_start:
        ret

add_x_y:
    mov al, byte [x_flag]
    mov ah, byte [y_flag]
    cmp al, ah
    jnz add_different_flag 
    add_same_flag:
        call    add_x_y_same_flag
        ret
    add_different_flag:
        call    add_x_y_different_flag
        ret    

;===============================================
; compare x and y,                             =
; al==2 if x>y, al==1 if x==y, otherwise al==0 =
;===============================================
compare_x_y:
    call    get_x_y_end
    call    get_x_y_start
    push    rbx
    push    rbp
    sub rbx, r8
    sub rbp, r9
    cmp rbx, rbp
    pop rbp
    pop rbx
    jz  compare_loop    ; the same length
    ja  x_bigger_than_y
    y_bigger_than_x:
        mov al, 0
        ret
    x_bigger_than_y:
        mov al, 2
        ret
    x_equal_to_y:
        mov al, 1
        ret
    compare_loop:
        cmp r8, rbx
        jb  x_equal_to_y
        mov al, byte [r8]
        cmp al, byte [r9]
        inc r8
        inc r9
        jz  compare_loop
        ja  x_bigger_than_y
        jb  y_bigger_than_x


add_x_y_different_flag:
    call    compare_x_y
    call    get_x_y_end
    call    get_x_y_start
    cmp al, 1
    jz  print_zero
    ja  finish_swap_x_y
    ; swap x and y
    mov rax, 0
    mov al, byte [x_flag]
    push    rax
    push    r8
    push    rbx
    mov al, byte [y_flag]
    push    rax
    push    r9
    push    rbp
    pop rbx
    pop r8
    pop rax
    mov byte [x_flag], al
    pop rbp
    pop r9
    pop rax
    mov byte [y_flag], al
    ; now x>y, and flag of result equals to x_flag
    finish_swap_x_y:
        mov al, 0   ; 作为借位
        mov rcx, add_result
    different_flag_add_loop:
        cmp rbx, r8
        jb  end_different_flag_add_loop
        mov ah, byte [rbx]
        cmp rbp, r9
        jb  skip_sub
        sub ah, byte [rbp]
        add ah, '0'
        skip_sub:
        sub ah, al
        mov al, 0
        cmp ah, '0'
        dec rbp
        dec rbx
        mov byte [rcx], ah
        inc rcx
        jae different_flag_add_loop
        add ah, 10
        mov al, 1
        dec rcx
        mov byte [rcx], ah
        inc rcx
        jmp different_flag_add_loop
    end_different_flag_add_loop:
        mov rcx, x_flag
        mov bl, byte [rcx]
        call    print_ascii
        
        mov rcx, add_result
        push    0
        push_add_result_loop:
            cmp byte [rcx], 0
            jz  pop_add_result_loop
            mov rbx, 0
            mov bl, byte [rcx]
            push    rbx
            inc rcx
            jmp push_add_result_loop
        pop_add_result_loop:
            mov rbx, 0
            pop rbx
            cmp rbx, 0
            jz  end_pop_add_result_loop
            call    print_ascii
            jmp pop_add_result_loop
        end_pop_add_result_loop: 
            ret
    print_zero:
        mov bl, '0'
        call    print_ascii
        ret

add_x_y_same_flag:
    call    get_x_y_end
    call    get_x_y_start

    ; set flag 0
    mov rax, 0
    push    1
    add_x_y_loop:
        mov al, ah  ; add the flag as the initial value
        add_x_to_al:
            cmp rbx, r8  ; judge if rbx lower than x in memory
            jb  add_y_to_al ; no need to add x if true
            add al, byte [rbx]
            sub al, 48
        add_y_to_al:
            cmp rbp, r9
            jb  cal_flag
            add al, byte [rbp]
            sub al, 48
        cal_flag:
            add al, 48
            call    set_flag
        push    rax

        dec rbx
        dec rbp
        cmp rbx, r8
        jnb add_x_y_loop
        cmp rbp, r9
        jnb add_x_y_loop
    
    ; 把最后一个进位加上去
    cmp ah, 1
    jnz print_result_loop
    mov al, 49
    push    rax

    ; print result
    mov bl, byte [x_flag]
    call    print_ascii
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
    call    get_x_y_end
    call    get_x_y_start

    mul_x_y_loop:
        cmp r9, rbp
        ja  end_mul_x_y_loop
        push    rbx    ; save end address of x for restoring after mul
        mov ch, 0   ; set initial flag to zero
        mov rdx, mul_result    ; result of mul
        add dl, byte [counter]
        bit_mul_x_y_loop:
            cmp rbx, r8
            jb  end_bit_mul_x_y_loop
            mov al, byte [rbp]
            mov cl, byte [rbx]
            call    mul_get_result_and_flag
            add ah, ch  ;
            ; byte [rdx] may be bigger than 9
            cmp ah, 9
            jbe no_flag
            sub ah, 10
            inc al  ; add flag to al
            no_flag: 
                mov ch, al  ; add flag to ch for next loop
                add byte [rdx], ah
                cmp byte [rdx], '9'
                jbe no_flag_too
                sub byte [rdx], 10
                inc ch
                no_flag_too:
                dec rbx
                inc rdx
                jmp bit_mul_x_y_loop
        end_bit_mul_x_y_loop:
            ; write the last flag to mul_result
            add_last_flag:
                ; cmp ch, 0
                ; jz  end_add_last_flag
                add byte [rdx], ch
                cmp byte [rdx], '9'
                jbe end_add_last_flag
                sub byte [rdx], 10
                mov ch, 1
                inc rdx
                jmp add_last_flag
            end_add_last_flag:
                dec rbp
                inc byte [counter]
                pop rbx ; restore end address of x for next loop
                jmp mul_x_y_loop
    end_mul_x_y_loop:
        ; print flag
        mov al, byte [x_flag]
        mov ah, byte [y_flag]
        cmp al, ah
        jz end_print_mul_flag
        mov bl, '-'
        call    print_ascii

        end_print_mul_flag:
            mov rax, mul_result
            add rax, 50
        trim_zero_from_mul_result:
            cmp rax, mul_result
            jz  mul_same_flag
            cmp byte [rax], '0'
            jnz mul_same_flag
            dec rax
            jmp trim_zero_from_mul_result

        mul_same_flag:  
            ; print number part of mul_result
            cmp rax, mul_result
            jb  end_print_mul_result
            mov bl, byte [rax]
            push    rax
            call    print_ascii
            pop rax
            dec rax
            jmp mul_same_flag
        end_print_mul_result:    
    ret


;==================================
; 输入分别保存到al, cl              =
; 乘法计算，进位和结果分别保存到al, ah =
;==================================
mul_get_result_and_flag:
    mov ah, 0
    sub al, '0'
    sub cl, '0'
    mul cl
    mov cl, 10
    div cl
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
    x_flag: db  0   ; representing +
    y_flag: db  0   ; representing +
    add_result: times   24  db  0
    mul_result: times   50    db  48
    mul_tmp:    times   50    db  48  
    counter:    db  0


section	.bss
    x:  resb    24
    y:  resb    24
    pbuf:	resb	1
    mul_result_start:   resb    8
