    global  sprint

section .text
sprint:
    push    rbp
    mov rbp, rsp

    push    rbx

    ; 循环打印字符串
    mov rbx, rdi    ; 第一个参数地址
    sprint_loop:
        cmp byte [rbx], 0H  ; 判断是否打印结束
        jz  end_sprint_loop
        mov	rax, 1H
        mov	rdi, 1H
        mov	rdx, 1H
        mov	rsi, rbx
        syscall
        inc rbx
        jmp sprint_loop
    end_sprint_loop:
    pop rbx
    
    leave
    ret