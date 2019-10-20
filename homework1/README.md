# 2019操作系统实验（一）

## 1 实验内容

### 1.1 Hello OS

#### 1.1.1 运行截图

![](https://i.loli.net/2019/10/13/uLKp3OXozA6hnre.png)

#### 1.1.2 代码

##### 1.1.2.1 boot.asm

```asm
	org	07c00h
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	call	DispStr
	jmp	$
DispStr:
	mov	ax, BootMessage
	mov	bp, ax
	mov	cx, 16
	mov	ax, 01301h
	mov	bx, 000ch
	mov	dl, 0
	int	10h
	ret
BootMessage:	db	"Hello, OS world!"
times	510-($-$$)	db	0
dw	0xaa55
```

##### 1.1.2.2 bochsrc

```ini
megs:32

display_library: sdl

#romimage: file=/usr/share/bochs/BIOS-bochs-latest
#vgaromimage: file=/usr/share/vgabios/vgabios.bin

floppya:1_44=a.img, status=inserted

boot:floppy

log: bochsout.txt

mouse: enabled=0

#keyboard_mapping: enabled=1, map=/usr/share/bochs/keymaps/x11-pc-us.map
```

### 2.1 汇编语言实践

#### 2.1.1 运行截图

##### 2.1.1.1 不含负数

![](https://i.loli.net/2019/10/13/3Z1W9J5IRzoGhgb.png)

##### 2.1.1.2 含负数

![](https://i.loli.net/2019/10/13/jTClhvg1sarX49t.png)

#### 2.1.2 代码

```asm
	global	_start

section .text
_start:
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
        ja  x_equal_to_y
        mov al, byte [r8]
        mov cl, byte [r9]
        ; cmp al, byte [r9]
        inc r8
        inc r9
        cmp al, cl
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
            jz  trim_zero_from_add_result
            mov rbx, 0
            mov bl, byte [rcx]
            push    rbx
            inc rcx
            jmp push_add_result_loop
        trim_zero_from_add_result:
            mov rbx, 0
            pop rbx
            cmp bl, '0'
            jz  trim_zero_from_add_result
            push    rbx
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
    
    ; check if both x and y are zero
    cmp r8, rbx
    jnz either_x_or_y_zero
    cmp r9, rbp
    jnz either_x_or_y_zero
    cmp byte [r8], '0'
    jnz either_x_or_y_zero
    cmp byte [r9], '0'
    jnz either_x_or_y_zero
    mov cl, 0
    either_x_or_y_zero:

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
    jnz print_add_flag
    mov al, 49
    push    rax

    ; print result
    print_add_flag:
        cmp cl, 0
        jz  print_result_loop
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
    
    ; check if both x and y are zero
    cmp r8, rbx
    jnz x_not_zero
    cmp byte [r8], '0'
    jnz x_not_zero
    mov bl, '0'
    call    print_ascii
    ret
    x_not_zero:
    cmp r9, rbp
    jnz both_x_and_y_not_zero
    cmp byte [r9], '0'
    jnz both_x_and_y_not_zero
    mov bl, '0'
    call    print_ascii
    ret
    both_x_and_y_not_zero:

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
```



## 2 问题清单

1. 请简述 80x86 系列的发展历史

     > **x86**泛指一系列[英特尔](https://zh.wikipedia.org/wiki/英特爾)公司用于开发处理器的[指令集架构](https://zh.wikipedia.org/wiki/指令集架構)，这类处理器最早为1978年面市的“[Intel 8086](https://zh.wikipedia.org/wiki/Intel_8086)”[CPU](https://zh.wikipedia.org/wiki/CPU)。
     >
     > 该系列较早期的处理器名称是以数字来表示**80x86**。由于以“86”作为结尾，包括[Intel 8086](https://zh.wikipedia.org/wiki/Intel_8086)、[80186](https://zh.wikipedia.org/wiki/80186)、[80286](https://zh.wikipedia.org/wiki/80286)、[80386](https://zh.wikipedia.org/wiki/80386)以及[80486](https://zh.wikipedia.org/wiki/80486)，因此其架构被称为“x86”。
     >
     > ......
     >
     > x86架构于1978年推出的[Intel 8086](https://zh.wikipedia.org/wiki/Intel_8086)[中央处理器](https://zh.wikipedia.org/wiki/中央處理器)中首度出现，它是从[Intel 8008](https://zh.wikipedia.org/wiki/Intel_8008)处理器中发展而来的，而8008则是发展自[Intel 4004](https://zh.wikipedia.org/wiki/Intel_4004)的。8086在三年后为[IBM PC](https://zh.wikipedia.org/wiki/IBM_PC)所选用，之后x86便成为了个人计算机的标准平台，成为了历来最成功的CPU架构。
     >
     > 8086是16位处理器；直到1985年32位的[80386](https://zh.wikipedia.org/wiki/80386)的开发，这个架构都维持是16位。接着一系列的处理器表示了32位架构的细微改进，推出了数种的扩展，直到2003年[AMD](https://zh.wikipedia.org/wiki/AMD)对于这个架构发展了64位的扩展，并命名为[AMD64](https://zh.wikipedia.org/wiki/AMD64)。后来英特尔也推出了与之兼容的处理器，并命名为[Intel 64](https://zh.wikipedia.org/wiki/Intel_64)。两者一般被统称为**x86-64**或**x64**，开创了x86的64位时代。
     >
     > —— wikipedia - x86

2. 说明小端和大端的区别，并说明 80x86 系列采用了哪种方式？

     采用大小模式对数据进行存放的主要区别在于在存放的字节顺序，大端方式将高位存放在低地址，小端方式将低位存放在高地址

     > ​		大端模式，是指数据的高位，保存在内存的低地址中，而数据的低位，保存在内存的高地址中，这样的存储模式有点儿类似于**把数据当作字符串顺序处理**：地址由小向大增加，而数据从高位往低位放；
     > ​		小端模式，是指数据的高位保存在内存的高地址中，而数据的低位保存在内存的低地址中，这种存储模式将地址的高低和数据位权有效地结合起来，高地址部分权值高，低地址部分权值低，和我们的逻辑方法一致。
     > ————————————————
     > 版权声明：本文为CSDN博主「花果山总钻风」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
     > 原文链接：https://blog.csdn.net/dszgf5717/article/details/38261391

     80x86使用的是**小端模式**，测试程序和运行结果如下：

     ![](https://i.loli.net/2019/10/14/9SJdKfrvYx8QVmA.png)

     ![](https://i.loli.net/2019/10/14/uir8AJM6nPkc4Ug.png)

3. 8086 有哪五类寄存器，请分别举例说明其作用？

     ![](https://i.loli.net/2019/10/14/YJ3ErDvFtzATSmX.png)

     ![](https://i.loli.net/2019/10/14/QO26XUptCf1Edy9.png)

4. 什么是寻址？立即寻址和直接寻址的区别是什么？

     表示指令中操作数所在的方法称为寻址方式（寻址就是定位指令中的操作数）

     - 立即寻址：操作数就包含在指令中，作为指令的一部分，操作数通常存放在数据段
     - 直接寻址：指令中存放的是操作数的有效地址，操作数存放在存储器中

5. 请举例说明寄存器间接寻址、寄存器相对寻址、基址加变址寻址、相对基址加变址寻址四
     种方式的区别

     - 寄存器间接寻址
     
       操作数在存储器中，操作数的**有效地址在SI、DI、BX、BP这四个寄存器之一中**；一般情况下，如果有效地址在SI、DI、BX中，则以DS段寄存器的内容为段值，否则如果有效地址在BP中，则以SS段寄存器的内容为段值
     
       ```asm
       ; (DS)=5000H, (SI)=1234H
       MOV	AX, [SI]
       ```
     
       那么AX的内容是地址51234H处的值
     
     - 寄存器相对寻址
     
       操作数在存储器中，操作数的有效地址是一个**基址寄存器（BX、BP）**或**变址寄存器（SI、DI）**内容加上**指令给定的**8位或16位位移量之和；一般情况下，如果SI、DI、BX的内容作为有效地址的一部分，则段寄存器是DS，否则如果BP的内容作为有效地址的一部分，则段寄存器的是SS
     
       ![](https://i.loli.net/2019/10/20/MxhuDvWAmnafNC6.png)
     
       ```asm
       ; (DS)=5000H, (DI)=3678H
       MOV	AX, [DI+1223H]
       ```
     
       那么AX的内容是地址5489BH处的值
     
     - 基址加变址寻址
     
       操作数在存储器中，操作数的有效地址由**基址寄存器（BX、BP）之一的内容**与**变址寄存器（SI、DI）之一的内容**相加得到；一般情况下，如果BP作为有效地址的一部分，那么段寄存器是SS，否则如果BX作为有效地址的一部分，那么段寄存器是DS
     
       ![](https://i.loli.net/2019/10/20/2IqbhtKxCj63M4G.png)
     
       ```asm
       ; (DS)=5000H, (BX)=1223H, (DI)=54H
       MOV	AX, [BX+DI]
       ```
     
       那么AX的内容是地址51277H处的值
     
     - 相对基址加变址寻址
     
       操作数在存储器中，操作数的有效地址由**基址寄存器之一的内容**与**变址寄存器之一的内容**及**指令中给定的**8位或16位位移量相加得到；一般情况下，如果BP作为有效地址的一部分，那么段寄存器是SS，否则如果BX作为有效地址的一部分，那么段寄存器是DS
     
       ![](https://i.loli.net/2019/10/20/jgUrwI92OY7MSly.png)
     
       ```asm
       ; (DS)=5000H, (BX)=1223H, (DI)=54H
       MOV	AX, [BX+DI-2]
       ```
     
       那么AX的内容是地址51275H处的值
     
6. 请分别简述 MOV 指令和 LEA 指令的用法和作用？

     - **mov**：数据传送指令

       ```asm
       mov eax, ebx	; CPU内部寄存器之间的数据传输
       mov ecx, 109	; 立即数传送至通用寄存器
       mov al, bl	; 寄存器之间的数据传送
       mov byte[var1], al	; 寄存器与存储器之间的数据传送
       mov word[var2], 200	; 立即数传送至存储单元
       mov eax, dword[var3]	; 寄存器与存储器之间的数据传送
       ```

       但是要注意：

       （1） MOV指令中的源操作数绝对不能是立即数和代码段CS寄存器； 
       （2） MOV指令中绝对不允许在两个存储单元之间直接传送数据； 
       （3） MOV指令中绝对不允许在两个段寄存器之间直接传送数据； 
       （4） MOV指令不会影响标志位

     - **lea**：load effective address，加载有效地址，可以将有效地址传送到指定的的寄存器。指令形式是从存储器读数据到寄存器, 效果是将存储器的有效地址写入到目的操作数

       ```asm
       LEA	REG, OPRD
       ```

       操作数OPRD必须是一个存储器操作数，操作数REG必须是一个通用寄存器，例如：

       ```asm
       LEA	AX, BUFFER
       LEA	DX, [BX+3]
       LEA	SI, [BP+DI+4]
       ```

     **两者使用[]区别**

     第二操作数加不加中括号[]的区别就是:

     - lea对变量没有影响是取地址,对寄存器来说加[]时取值,第二操作数不加[]非法

     - mov对变量来说没有影响是取值,对寄存器来说是加[]时取地址,第二操作数不加[]是取值

7. 请说出主程序与子程序之间至少三种参数传递方式

     - 利用寄存器传递参数

       把参数放在约定的寄存器中，优点是实现简单和调用方便，但是由于寄存器个数有限，且寄存器还要存放其他数据，所以只适合传递参数较少的情况

     - 利用约定存储单元传递参数

       传递参数较多的情况下，可利用约定的内存变量来传递参数，优点是子程序要处理的数据或送出的结果都有独立的存储单元，编程时不易出错

     - 利用堆栈传递参数

       如果使用堆栈传递入口参数，那么主程序在调用子程序之前，把需要传递的参数依次压入堆栈，子程序从堆栈中取入口参数；如果使用堆栈传递出口参数，那么子程序在返回之前，把需要返回的参数存入堆栈，主程序在堆栈中取出参数

     - 利用CALL后续区传递参数

8. 如何处理输入和输出，代码中哪里体现出来？

     ```asm
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
     	mov	rax, 1H	; syscall number
     	mov	rdi, 1H	; unsigned int fd, stdout
     	mov	rdx, 1H	; size_t count
     	mov	rsi, pbuf	; const char *buf
     	syscall
         ret
     
     ;=============================
     ; read a char from stdin     =
     ; the input char saved in bl =
     ;=============================
     get_char:
     	;use `pbuf` as input buffer
     	mov	rax, 0H	; syscall number
     	mov	rdi, 0H	; unsigned int fd, stdin
     	mov	rsi, pbuf	; char *buf
     	mov	rdx, 1H	;read one char
     	syscall
     	mov	bl, [pbuf]	;save char in register `bl`
         ret
         
     section	.data
         hint:	db	"Please input x and y: "
     section	.bss
         pbuf:	resb	1
     ```

     系统调用表：

     | rax  | System Call |       rdi       |       rsi       |     rdx      |
     | :--: | :---------: | :-------------: | :-------------: | :----------: |
     |  0   |  sys_read   | unsigned int fd |    char *buf    | size_t count |
     |  1   |  sys_write  | unsigned int fd | const char *buf | size_t count |

9. 有哪些段寄存器

     每当需要产生一个20位的物理地址时，BIU会自动引用一个段寄存器，且左移4位再与一个16位的偏移量相加

     ![](https://i.loli.net/2019/10/20/YOgDzojBlc84bKt.png)

     ![](https://i.loli.net/2019/10/20/xyjvSo5U6qbJzP1.png)

     - CS：代码段寄存器

       例如在取指令时，自动引用代码段寄存器CS，再加上IP所给出的16位偏移，得到要取的指令的物理地址

     - SS：堆栈段寄存器

       - 当涉及一个堆栈操作时，则自动引用SS，再加上SP所给出的16位偏移得到堆栈操作所需的物理地址
       - 当偏移涉及BP寄存器时，缺省引用的段寄存器也是SS

     - DS：数据段寄存器

     - ES：附加段寄存器

     ![](https://i.loli.net/2019/10/20/aD5uTQUFBrgfRHk.png)

10. 通过什么寄存器保存前一次的运算结果，在代码中哪里体现出来。

     

11. 解释 boot.asm 文件中， org 0700h 的作用

      Org是伪指令，伪指令是指， 不生成对应的二进制指令， 只是汇编器使用的。

      它的作用是：**告诉汇编器**， 当前这段代码会放在07c00h处。 所以， 如果之后遇到需要绝对寻址地指令， 那么绝对地址就是07c00h加上相对地址

      > - 绝对地址： 内存的实际位置（ 先不考虑内存分页一类逻辑地址）
      > - 相对地址： 当前指令相对第一行代码的位置

      在第一行加上org 07c00h只是让编译器从相对地址07c00h处开始编译第一条指令， 相对地址被编译加载后就正好和绝对地址吻合

12. boot.bin 应该放在软盘的哪一个扇区？为什么？

      需要把boot.bin放在软盘的**第一个扇区**，原因：

      **BIOS**

      - 开机， 从ROM运行BIOS程序， BIOS是厂家写好的
      - BIOS程序检查**软盘0面0磁道1扇区**， 如果扇区**以0xaa55结束**， 则认定为**引导扇区**， 将其**512字节**的数据**加载到内存的07c00处**， 然后设置PC， 跳到**内存07c00处开始执行代码**
      - 以上的0xaa55以及07c00都是一种约定， BIOS程序就是这样做的， 所以我们就需要**把我们的os放在软盘的第一个扇区， 填充， 并在最末尾写入0xaa55**

13. loader 的作用有哪些？

      - **由loader将内核kernel加载入内存**，由于我们是在操作系统层面上编写另一个操作系统，于是生成的内核可执行文件是和当前操作系统平台相关的，比如linux下是elf格式，有许多无关信息， 于是， 内核并不能像boot.bin或loader.bin那样直接放入内存中， **需要loader从kernel.bin中提取出需要放入内存中的部分**
      - 简单来说，Loader程序就是从软盘的根目录将内核文件kernel.dll载入物理内存0x10000，然后通过**开启分页机制**，映射到虚拟地址0x80000000处，然后**Loader程序跳转到kernel.dll的入口点继续执行**，到此，控制权交给了内核
      - Loader 程序还负责检测内存大小，**为内核准备保护模式执行环境**等工作，最后从实模式跳转到保护模式，使得能够访问1M以上的内存

14. 解释 NASM 语言中 [ ] 的作用

      **解引用。**例如：

      ```asm
      MOV dword[ebx], 1
      INC BYTE[label]
      ADD eax, dword[label]
      ```

      []中寄存器的值或者标签、变量（<u>NASM中标签和变量是一样的，例如`foo dw 1`等价于`foo: dw 1`</u>）是一个有效地址，[]的作用是取出有效地址（或者说指针）指向的值。[]之前可以有如下标识来指示要取出的位数：

      ```asm
      BYTE, WORD, DWORD, QWORD, TWORD
      ```

15. 解释语句 times 510-(\$-\$\$) db 0，为什么是 510? \$ 和 $$ 分别表示什么？

      ```asm
      	org	07c00h
      	mov	ax, cs
      	mov	ds, ax
      	mov	es, ax
      	call	DispStr
      	jmp	$
      DispStr:
      	mov	ax, BootMessage
      	mov	bp, ax
      	mov	cx, 16
      	mov	ax, 01301h
      	mov	bx, 000ch
      	mov	dl, 0
      	int	10h
      	ret
      BootMessage:	db	"Hello, OS world!"
      times	510-($-$$)	db	0
      dw	0xaa55
      ```

      - `$`表示当前行被汇编后的地址
      - `$$`表示一个节（section）的开始处被汇编后的地址。在这里我们的程序只有1个节，所以`$$`是实际上就表示程序被汇编后的开始地址，即0x7c00
      - `$-$$`就表示本行距离程序开始处的相对地址
      - `times	510-($-$$)	db	0`后面还有一条语句是`dw 0xaa55`，说明是将0xaa55（占用两个字节）写到511-512这两个地址处，恰好是2个字节，因此应该使用0来填充`times	510-($-$$)	db	0`～510地址处

16. 解释配置文件 bochsrc 文件中各参数的含义

        ```ini
        megs:32
        display_library: sdl
        floppya: 1_44=a.img, status=inserted
        boot: floppy
        ```

      display_library: bochs使用的GUI库， 在Ubuntu下面是sdl
      megs： 虚拟机内存大小 (MB)
      floppya： 虚拟机外设， 软盘为a.img文件
      boot： 虚拟机启动方式， 从软盘启动